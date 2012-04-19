//#include <Python.h>
#include "raster.h"
#include "rasterFriends.h"

void adjustCroplandProbabilityLayer(raster & inAreaRaster,
									raster & inCountriesRaster,
									raster & inClassRaster,
									raster & outClassRaster,
									const runParamsT & params,
									agreementTableT & agTable
									)
{
	printf("Adjusting cropland probability classes\n");

	map< float, float * > classesByCountryTable;	// key - country ID, value - vector of areas for all classes
	map< float, vector<int> > oldToNewClassesMap;	// key - country ID, value - vector of new (adjusted) classes

	inAreaRaster.validateExtent(inCountriesRaster);
	inAreaRaster.validateExtent(inClassRaster);

	raster::statisticsStructT probabilityStatistics = inClassRaster.describe();
	size_t maxClass = (size_t)probabilityStatistics.maxVal;
	size_t minClass = (size_t)probabilityStatistics.minVal;

	vector<int> dummyVector(maxClass+1, -999);

	ifstream inAreaFile;
	inAreaFile.open(inAreaRaster.getFltPath().c_str(), ios::in | ios::binary);
	ASSERT_INT(inAreaFile.is_open(), FILE_NOT_OPEN);
	ifstream inCountriesFile;
	inCountriesFile.open(inCountriesRaster.getFltPath().c_str(), ios::in | ios::binary);
	ASSERT_INT(inCountriesFile.is_open(), FILE_NOT_OPEN);
	ifstream inClassFile;
	inClassFile.open(inClassRaster.getFltPath().c_str(), ios::in | ios::binary);
	ASSERT_INT(inClassFile.is_open(), FILE_NOT_OPEN);
	ofstream outClassFile;
	outClassFile.open(outClassRaster.getFltPath().c_str(), ios::out | ios::binary);
	
	inAreaRaster.copyFile(inAreaRaster.getHdrPath(), outClassRaster.getHdrPath());
	inAreaRaster.copyProperties(outClassRaster);

	long countriesBegin = inCountriesFile.tellg();
	long classBegin = inClassFile.tellg();

	int numCells = inAreaRaster.horResolution * inAreaRaster.verResolution;
	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);

	float * bufArea = new float[bufSize];
	float * bufCountries = new float[bufSize];
	float * bufClass = new float[bufSize];
	float * outBufClass = new float[bufSize];

	printf("Computing statistics for classes per country\n");
	int numCellsProcessed = 0;

	// First run - collecting statistics
	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		numCellsProcessed += bufSize;

		inAreaFile.read(reinterpret_cast<char*>(bufArea), sizeof(float) * bufSize);
		inCountriesFile.read(reinterpret_cast<char*>(bufCountries), sizeof(float) * bufSize);
		inClassFile.read(reinterpret_cast<char*>(bufClass), sizeof(float) * bufSize);

		for (int i = 0; i < bufSize; i++)
		{
			if ((bufClass[i] != inClassRaster.noDataValue) &&
				(bufCountries[i] != inCountriesRaster.noDataValue) &&
				(bufArea[i] != inAreaRaster.noDataValue))
			{
				map< float, float * >::iterator it = classesByCountryTable.find(bufCountries[i]);
				if (it == classesByCountryTable.end())
				{
					printf("  > adding: c = %d\n", (int)bufCountries[i]);
					float * tmp = new float[maxClass + 1];
					memset(tmp, 0, sizeof(float) * (maxClass + 1));
					classesByCountryTable.insert(make_pair<float, float *>(bufCountries[i], tmp));
					oldToNewClassesMap.insert(make_pair<float, vector<int> >(bufCountries[i], dummyVector));
					/*map< float, float * >::iterator */it = classesByCountryTable.find(bufCountries[i]);
				}
				it->second[(int)bufClass[i]] += bufArea[i];
			}
		}
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}
	
	// Second run through the table - rearrange classes
	map< float, float * >::iterator it = classesByCountryTable.begin();
	while (it != classesByCountryTable.end())
	{
		//for (int x = 0; x < maxClass; x++)
		//{
		//	printf("%f  ", it->second[x]);
		//}
		//printf("\n");
		
		for (size_t i = 0; i < maxClass; i++)
		{
			oldToNewClassesMap[it->first][i] = -1;
		}

		for (size_t i = minClass; i < maxClass; i++)
		{
			// it->first - current country ID
			// i - investigated class
			// it->second[i] - area of this class
			// agTable.similarClassesMatrix[i][:] - vector saying true for classes similar to 
			oldToNewClassesMap[it->first][i] = (oldToNewClassesMap[it->first][i] < 0) ? (int)i : oldToNewClassesMap[it->first][i];

			float swp = it->second[i];
			int swpClass = i;
			bool swpFl = false;

			size_t j = i + 1;
			while ((j <= maxClass) && agTable.checkSimilarity(i, j))
			{
				if (it->second[j] > swp)
				{
					swp = it->second[j];
					swpClass = j;
					swpFl = true;
				}
				j++;
			}
			
			if (swpFl)
			{
				it->second[swpClass] = it->second[i];
				it->second[i] = swp;
				int subs = oldToNewClassesMap[it->first][i];
				oldToNewClassesMap[it->first][i] = (oldToNewClassesMap[it->first][swpClass] < 0)
					? (int)swpClass
					: oldToNewClassesMap[it->first][swpClass];
				oldToNewClassesMap[it->first][swpClass] = subs;
			}

			//printf(" > (%d) ", i);
			//for (int x = 0; x < maxClass; x++)
			//{
			//	printf("%d  ", oldToNewClassesMap[it->first][x]);
			//}
			//printf("\n");
		}
		//printf("==========\n");
		//for (int x = 0; x < maxClass; x++)
		//{
		//	printf("%d  ", oldToNewClassesMap[it->first][x]);
		//}
		//printf("\n");
		//for (int x = 0; x < maxClass; x++)
		//{
		//	printf("%f  ", it->second[x]);
		//}
		//printf("\n");
		//system("pause");
		
		it++;
	}

	// Third run - write adjusted classes into the resulting raster
	inCountriesFile.seekg(countriesBegin);
	inClassFile.seekg(classBegin);
	
	printf("Writing results to the output file\n");
	numCellsProcessed = 0;
	bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);

	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		numCellsProcessed += bufSize;

		inCountriesFile.read(reinterpret_cast<char*>(bufCountries), sizeof(float) * bufSize);
		inClassFile.read(reinterpret_cast<char*>(bufClass), sizeof(float) * bufSize);

		for (int i = 0; i < bufSize; i++)
		{
			if ((bufClass[i] != inClassRaster.noDataValue) &&
				(bufCountries[i] != inCountriesRaster.noDataValue))
			{
				outBufClass[i] = (float)oldToNewClassesMap[bufCountries[i]][(int)bufClass[i]];
			}
			else
			{
				outBufClass[i] = outClassRaster.noDataValue;
			}
		}
		outClassFile.write(reinterpret_cast<char *>(outBufClass), sizeof(float) * bufSize);
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}	

	// Free up memory
	delete [] bufArea;
	delete [] bufCountries;
	delete [] bufClass;
	delete [] outBufClass;

	it = classesByCountryTable.begin();
	while (it != classesByCountryTable.end())
	{
		delete [] it->second;
		it++;
	}
	classesByCountryTable.clear();

	inAreaFile.close();
	inCountriesFile.close();
	inClassFile.close();
}

void validateCropland(raster & inCroplandRaster,
					  raster & inZoneRaster,
					  raster & inClassRaster,
					  raster & outCroplandRaster,
					  raster & outMinClassRaster)
{
	// Compute statistics
	raster::summaryTableT calibratedResults;
	inCroplandRaster.zonalSumByClassAsTable(inZoneRaster, inClassRaster, calibratedResults);

	// Write statistics to resulting files
	string inCroplandHdrPath = inCroplandRaster.rasterPath + ".hdr";
	string inCroplandFltPath = inCroplandRaster.rasterPath + ".flt";
	string inZoneHdrPath = inZoneRaster.rasterPath + ".hdr";
	string inZoneFltPath = inZoneRaster.rasterPath + ".flt";
	string inClassFltPath = inClassRaster.rasterPath + ".flt";
	string inClassHdrPath = inClassRaster.rasterPath + ".hdr";
	string outCroplandHdrPath = outCroplandRaster.rasterPath + ".hdr";
	string outCroplandFltPath = outCroplandRaster.rasterPath + ".flt";
	string outMinClassHdrPath = outMinClassRaster.rasterPath + ".hdr";
	string outMinClassFltPath = outMinClassRaster.rasterPath + ".flt";

	inCroplandRaster.copyFile(inZoneHdrPath, outCroplandHdrPath);
	inCroplandRaster.copyFile(inZoneHdrPath, outMinClassHdrPath);
	inCroplandRaster.copyProperties(outCroplandRaster);
	inCroplandRaster.copyProperties(outMinClassRaster);

	ifstream inCroplandFile;
	inCroplandFile.open(inCroplandFltPath.c_str(), ios::out | ios::binary);
	ifstream inZoneFile;
	inZoneFile.open(inZoneFltPath.c_str(), ios::out | ios::binary);
	ifstream inClassFile;
	inClassFile.open(inClassFltPath.c_str(), ios::out | ios::binary);
	ofstream outCroplandFile;
	outCroplandFile.open(outCroplandFltPath.c_str(), ios::out | ios::binary);
	ofstream outMinClassFile;
	outMinClassFile.open(outMinClassFltPath.c_str(), ios::out | ios::binary);

	int numCells = inCroplandRaster.horResolution * inCroplandRaster.verResolution;
	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);

	float * inBufCropland = new float[bufSize];
	float * inBufZone = new float[bufSize];
	float * inBufClass = new float[bufSize];
	float * outBufCropland = new float[bufSize];
	float * outBufMinClass = new float[bufSize];

	printf("Writing results to files\n");
	int numCellsProcessed = 0;
	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		numCellsProcessed += bufSize;
		inCroplandFile.read(reinterpret_cast<char*>(inBufCropland), sizeof(float) * bufSize);
		inZoneFile.read(reinterpret_cast<char*>(inBufZone), sizeof(float) * bufSize);
		inClassFile.read(reinterpret_cast<char*>(inBufClass), sizeof(float) * bufSize);
		for (int i = 0; i < bufSize; i++)
		{
			if ((inBufCropland[i] != inCroplandRaster.noDataValue)
				&& (inBufZone[i] != inZoneRaster.noDataValue)
				&& (inBufClass[i] != inClassRaster.noDataValue))
			{
				raster::unitResultT unitResult = calibratedResults[inBufZone[i]];
				outBufCropland[i] = (unitResult.bestClass <= inBufClass[i]) ? inBufCropland[i] : outCroplandRaster.noDataValue;
				outBufMinClass[i] = (unitResult.bestClass <= inBufClass[i]) ? unitResult.bestClass : outMinClassRaster.noDataValue;
			}
			else
			{
				if (inBufZone[i] != inZoneRaster.noDataValue)
				{
					raster::summaryTableT::iterator unitResultIter = calibratedResults.find(inBufZone[i]);
					if (unitResultIter != calibratedResults.end())
					{
						raster::unitResultT unitResult = unitResultIter->second;
						outBufMinClass[i] = (float)unitResultIter->second.bestClass;
					}
					else
					{
						outBufMinClass[i] = outMinClassRaster.noDataValue;
					}
				}
				else
				{
					outBufMinClass[i] = outMinClassRaster.noDataValue;
				}
				outBufCropland[i] = outCroplandRaster.noDataValue;
			}
		}
		outCroplandFile.write(reinterpret_cast<char *>(outBufCropland), sizeof(float) * bufSize);
		outMinClassFile.write(reinterpret_cast<char *>(outBufMinClass), sizeof(float) * bufSize);
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}

	inCroplandFile.close();
	inZoneFile.close();
	inClassFile.close();
	outCroplandFile.close();
	outMinClassFile.close();

	delete [] inBufCropland;
	delete [] inBufZone;
	delete [] inBufClass;
	delete [] outBufCropland;
	delete [] outBufMinClass;
}

void validateCropland(raster & inCroplandRaster,
					  raster & inZoneRaster,
					  raster & inClassRaster,
					  raster & outCroplandRaster,
					  raster & outMinClassRaster,
					  raster & outErrorRaster)
{
	// Compute statistics
	raster::summaryTableT calibratedResults;
	inCroplandRaster.zonalSumByClassAsTable(inZoneRaster, inClassRaster, calibratedResults);

	// Write statistics to resulting files
	string inCroplandHdrPath = inCroplandRaster.rasterPath + ".hdr";
	string inCroplandFltPath = inCroplandRaster.rasterPath + ".flt";
	string inZoneHdrPath = inZoneRaster.rasterPath + ".hdr";
	string inZoneFltPath = inZoneRaster.rasterPath + ".flt";
	string inClassFltPath = inClassRaster.rasterPath + ".flt";
	string inClassHdrPath = inClassRaster.rasterPath + ".hdr";
	string outCroplandHdrPath = outCroplandRaster.rasterPath + ".hdr";
	string outCroplandFltPath = outCroplandRaster.rasterPath + ".flt";
	string outMinClassHdrPath = outMinClassRaster.rasterPath + ".hdr";
	string outMinClassFltPath = outMinClassRaster.rasterPath + ".flt";
	string outErrorHdrPath = outErrorRaster.rasterPath + ".hdr";
	string outErrorFltPath = outErrorRaster.rasterPath + ".flt";

	inCroplandRaster.copyFile(inZoneHdrPath, outCroplandHdrPath);
	inCroplandRaster.copyFile(inZoneHdrPath, outErrorHdrPath);
	inCroplandRaster.copyFile(inZoneHdrPath, outMinClassHdrPath);
	inCroplandRaster.copyProperties(outCroplandRaster);
	inCroplandRaster.copyProperties(outErrorRaster);
	inCroplandRaster.copyProperties(outMinClassRaster);

	ifstream inCroplandFile;
	inCroplandFile.open(inCroplandFltPath.c_str(), ios::out | ios::binary);
	ifstream inZoneFile;
	inZoneFile.open(inZoneFltPath.c_str(), ios::out | ios::binary);
	ifstream inClassFile;
	inClassFile.open(inClassFltPath.c_str(), ios::out | ios::binary);
	ofstream outCroplandFile;
	outCroplandFile.open(outCroplandFltPath.c_str(), ios::out | ios::binary);
	ofstream outErrorFile;
	outErrorFile.open(outErrorFltPath.c_str(), ios::out | ios::binary);
	ofstream outMinClassFile;
	outMinClassFile.open(outMinClassFltPath.c_str(), ios::out | ios::binary);

	int numCells = inCroplandRaster.horResolution * inCroplandRaster.verResolution;
	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);

	float * inBufCropland = new float[bufSize];
	float * inBufZone = new float[bufSize];
	float * inBufClass = new float[bufSize];
	float * outBufCropland = new float[bufSize];
	float * outBufError = new float[bufSize];
	float * outBufMinClass = new float[bufSize];

	printf("Writing results to files\n");
	int numCellsProcessed = 0;
	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		numCellsProcessed += bufSize;
		inCroplandFile.read(reinterpret_cast<char*>(inBufCropland), sizeof(float) * bufSize);
		inZoneFile.read(reinterpret_cast<char*>(inBufZone), sizeof(float) * bufSize);
		inClassFile.read(reinterpret_cast<char*>(inBufClass), sizeof(float) * bufSize);
		for (int i = 0; i < bufSize; i++)
		{
			if ((inBufCropland[i] != inCroplandRaster.noDataValue)
				&& (inBufZone[i] != inZoneRaster.noDataValue)
				&& (inBufClass[i] != inClassRaster.noDataValue))
			{
				raster::unitResultT unitResult = calibratedResults[inBufZone[i]];
				outBufCropland[i] = (unitResult.bestClass <= inBufClass[i]) ? inBufCropland[i] : outCroplandRaster.noDataValue;
				outBufMinClass[i] = (unitResult.bestClass <= inBufClass[i]) ? unitResult.bestClass : outMinClassRaster.noDataValue;
				outBufError[i] = unitResult.error;
			}
			else
			{
				if (inBufZone[i] != inZoneRaster.noDataValue)
				{
					raster::summaryTableT::iterator unitResultIter = calibratedResults.find(inBufZone[i]);
					if (unitResultIter != calibratedResults.end())
					{
						raster::unitResultT unitResult = unitResultIter->second;
						outBufError[i] = unitResultIter->second.error;
						outBufMinClass[i] = (float)unitResultIter->second.bestClass;
					}
					else
					{
						outBufError[i] = outErrorRaster.noDataValue;
						outBufMinClass[i] = outMinClassRaster.noDataValue;
					}
				}
				else
				{
					outBufError[i] = outErrorRaster.noDataValue;
					outBufMinClass[i] = outMinClassRaster.noDataValue;
				}
				outBufCropland[i] = outCroplandRaster.noDataValue;
			}
		}
		outCroplandFile.write(reinterpret_cast<char *>(outBufCropland), sizeof(float) * bufSize);
		outErrorFile.write(reinterpret_cast<char *>(outBufError), sizeof(float) * bufSize);
		outMinClassFile.write(reinterpret_cast<char *>(outBufMinClass), sizeof(float) * bufSize);
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}

	inCroplandFile.close();
	inZoneFile.close();
	inClassFile.close();
	outCroplandFile.close();
	outErrorFile.close();
	outMinClassFile.close();

	delete [] inBufCropland;
	delete [] inBufZone;
	delete [] inBufClass;
	delete [] outBufCropland;
	delete [] outBufError;
	delete [] outBufMinClass;
}

void validateCropland(raster & inCroplandRaster,
					  raster & inZoneRaster,
					  raster & inClassRaster,
					  raster & outCroplandRaster)
{
	// Compute statistics
	raster::summaryTableT calibratedResults;
	inCroplandRaster.zonalSumByClassAsTable(inZoneRaster, inClassRaster, calibratedResults);

	// Write statistics to resulting files
	string inCroplandHdrPath = inCroplandRaster.rasterPath + ".hdr";
	string inCroplandFltPath = inCroplandRaster.rasterPath + ".flt";
	string inZoneHdrPath = inZoneRaster.rasterPath + ".hdr";
	string inZoneFltPath = inZoneRaster.rasterPath + ".flt";
	string inClassFltPath = inClassRaster.rasterPath + ".flt";
	string inClassHdrPath = inClassRaster.rasterPath + ".hdr";
	string outCroplandHdrPath = outCroplandRaster.rasterPath + ".hdr";
	string outCroplandFltPath = outCroplandRaster.rasterPath + ".flt";

	inCroplandRaster.copyFile(inZoneHdrPath, outCroplandHdrPath);
	inCroplandRaster.copyProperties(outCroplandRaster);

	ifstream inCroplandFile;
	inCroplandFile.open(inCroplandFltPath.c_str(), ios::out | ios::binary);
	ifstream inZoneFile;
	inZoneFile.open(inZoneFltPath.c_str(), ios::out | ios::binary);
	ifstream inClassFile;
	inClassFile.open(inClassFltPath.c_str(), ios::out | ios::binary);
	ofstream outCroplandFile;
	outCroplandFile.open(outCroplandFltPath.c_str(), ios::out | ios::binary);

	int numCells = inCroplandRaster.horResolution * inCroplandRaster.verResolution;
	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);

	float * inBufCropland = new float[bufSize];
	float * inBufZone = new float[bufSize];
	float * inBufClass = new float[bufSize];
	float * outBufCropland = new float[bufSize];

	printf("Writing results to files\n");
	int numCellsProcessed = 0;
	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		numCellsProcessed += bufSize;
		inCroplandFile.read(reinterpret_cast<char*>(inBufCropland), sizeof(float) * bufSize);
		inZoneFile.read(reinterpret_cast<char*>(inBufZone), sizeof(float) * bufSize);
		inClassFile.read(reinterpret_cast<char*>(inBufClass), sizeof(float) * bufSize);
		for (int i = 0; i < bufSize; i++)
		{
			if ((inBufCropland[i] != inCroplandRaster.noDataValue)
				&& (inBufZone[i] != inZoneRaster.noDataValue)
				&& (inBufClass[i] != inClassRaster.noDataValue))
			{
				raster::unitResultT unitResult = calibratedResults[inBufZone[i]];
				outBufCropland[i] = (unitResult.bestClass <= inBufClass[i]) ? inBufCropland[i] : inZoneRaster.noDataValue;
			}
			else
			{
				if (inBufZone[i] != inZoneRaster.noDataValue)
				{
					raster::summaryTableT::iterator unitResultIter = calibratedResults.find(inBufZone[i]);
					if (unitResultIter != calibratedResults.end())
					{
						raster::unitResultT unitResult = unitResultIter->second;
					}
				}
				outBufCropland[i] = outCroplandRaster.noDataValue;
			}
		}
		outCroplandFile.write(reinterpret_cast<char *>(outBufCropland), sizeof(float) * bufSize);
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}

	inCroplandFile.close();
	inZoneFile.close();
	inClassFile.close();
	outCroplandFile.close();

	delete [] inBufCropland;
	delete [] inBufZone;
	delete [] inBufClass;
	delete [] outBufCropland;
}

void getCalibratedZones(const vector<float> & valVector,
						const vector<float> & noDataValuesVector,
						const vector<float> & noDataValuesOutVector,
						vector<float> & result)
{
	float resultLevel = valVector[0];
	float sumLevel = valVector[1];
	float sumLevelUp = valVector[2];

	if ((sumLevel == noDataValuesVector[1]) && (sumLevelUp == noDataValuesVector[2]))
	{
		result[0] = noDataValuesOutVector[0];
		result[1] = noDataValuesOutVector[1];
	}
	else
	{
		// get cells of cropland from results of current level
		result[0] = ((sumLevel >= sumLevelUp) && (resultLevel != noDataValuesVector[0])) ? resultLevel : noDataValuesOutVector[0];
		// get mask of cells for which calibration is needed
		result[1] = (sumLevel < sumLevelUp) ? 1 : noDataValuesOutVector[1];
	}
}

void combineLevels(const vector<float> & valVector,
				   const vector<float> & noDataValuesVector,
				   const vector<float> & noDataValuesOutVector,
				   vector<float> & result)
{
	float zonesToCalibrate = valVector[0];
	float resultCalibrated = valVector[1];
	float resultLevel = valVector[2];

	result.resize(1);

	if (compare_eq(zonesToCalibrate, 1, EPSILON))
	{
		if (!compare_eq(resultCalibrated, noDataValuesVector[1], EPSILON))
		{
			result[0] = resultCalibrated;
		}
		else 
		{
			result[0] = noDataValuesOutVector[0];
		}
	}
	else
	{
		if (!compare_eq(resultLevel, noDataValuesVector[2], EPSILON))
		{
			result[0] = resultLevel;
		}
		else 
		{
			result[0] = noDataValuesOutVector[0];
		}
	}
}

float selectAreaByMask(float val, float mask)
{
	return xmax(val, (float)0);
}

void calibrateCropland(raster & inCroplandRaster,
					   raster & inClassRaster,
					   raster & statisticsLevelUpRaster,
					   raster & statisticsLevelRaster,
					   raster & resultLevelUpRaster,
					   raster & resultLevelRaster,
					   raster & outCalibratedRasterLevel,
					   const runParamsT & runParams)
{
	// Temporary rasters are not deleted in debug mode
	raster sumLevelUpRaster(runParams.tmpDir + "sum_level_up", raster::TEMPORARY);		// sum of cropland per administrative units for results at the above level
	raster sumLevelRaster(runParams.tmpDir + "sum_level", raster::TEMPORARY);			// sum of cropland per administrative units for results at the current level
	raster levelResultsToUseRaster(runParams.tmpDir + "level_results_to_use", raster::TEMPORARY);
	raster levelSumToUseRaster(runParams.tmpDir + "level_sum_to_use", raster::TEMPORARY);
	raster zonesToCalibrateRaster(runParams.tmpDir + "zones_to_calibrate", raster::TEMPORARY);
	raster differenceRaster(runParams.tmpDir + "difference", raster::TEMPORARY);
	raster calibratedStatisticsLevelRaster(runParams.tmpDir + "calibrated_statistics_level", raster::TEMPORARY); 	// Calibrated statistics for level 1
	raster resultsForCalibratedZonesRaster(runParams.tmpDir + "results_for_calibrated_zones", raster::TEMPORARY); 	// Calibrated statistics for level 1

	resultLevelRaster.zonalStatistics(statisticsLevelRaster, sumLevelRaster, raster::SUM);
	resultLevelUpRaster.zonalStatistics(statisticsLevelRaster, sumLevelUpRaster, raster::SUM);

	vector<raster *> passVector;
	passVector.push_back(&resultLevelRaster);
	passVector.push_back(&sumLevelRaster);
	passVector.push_back(&sumLevelUpRaster);

	vector<raster *> getBackVector;
	getBackVector.push_back(&levelResultsToUseRaster);
	getBackVector.push_back(&zonesToCalibrateRaster);

	multipleRasterArithmetics(&getCalibratedZones, passVector, getBackVector);
	levelResultsToUseRaster.zonalStatistics(statisticsLevelUpRaster, levelSumToUseRaster, raster::SUM);
	statisticsLevelUpRaster.rasterArithmetics(&xminus, levelSumToUseRaster, differenceRaster);
	differenceRaster.rasterArithmetics(&selectAreaByMask, zonesToCalibrateRaster, calibratedStatisticsLevelRaster);

	validateCropland(inCroplandRaster,
		calibratedStatisticsLevelRaster,
		inClassRaster,
		resultsForCalibratedZonesRaster);

	passVector.clear();
	passVector.push_back(&zonesToCalibrateRaster);
	passVector.push_back(&resultsForCalibratedZonesRaster);
	passVector.push_back(&resultLevelRaster);

	getBackVector.clear();
	getBackVector.push_back(&outCalibratedRasterLevel);

	multipleRasterArithmetics(&combineLevels, passVector, getBackVector);

	printf(__TIME__ "\n");
}

void calibrateCropland(raster & inCroplandRaster,
					   raster & inClassRaster,
					   raster & statisticsLevelUpRaster,
					   raster & statisticsLevelRaster,
					   raster & resultLevelUpRaster,
					   raster & resultLevelRaster,
					   raster & outCalibratedRasterLevel,
					   raster & outMinClassRaster,
					   const runParamsT & runParams)
{
	// Temporary rasters are not deleted in debug mode
	raster sumLevelUpRaster(runParams.tmpDir + "sum_level_up", raster::TEMPORARY);		// sum of cropland per administrative units for results at the above level
	raster sumLevelRaster(runParams.tmpDir + "sum_level", raster::TEMPORARY);			// sum of cropland per administrative units for results at the current level
	raster levelResultsToUseRaster(runParams.tmpDir + "level_results_to_use", raster::TEMPORARY);
	raster levelSumToUseRaster(runParams.tmpDir + "level_sum_to_use", raster::TEMPORARY);
	raster zonesToCalibrateRaster(runParams.tmpDir + "zones_to_calibrate", raster::TEMPORARY);
	raster differenceRaster(runParams.tmpDir + "difference", raster::TEMPORARY);
	raster calibratedStatisticsLevelRaster(runParams.tmpDir + "calibrated_statistics_level", raster::TEMPORARY); 	// Calibrated statistics for level 1
	raster resultsForCalibratedZonesRaster(runParams.tmpDir + "results_for_calibrated_zones", raster::TEMPORARY); 	// Calibrated statistics for level 1

	resultLevelRaster.zonalStatistics(statisticsLevelRaster, sumLevelRaster, raster::SUM);
	resultLevelUpRaster.zonalStatistics(statisticsLevelRaster, sumLevelUpRaster, raster::SUM);

	vector<raster *> passVector;
	passVector.push_back(&resultLevelRaster);
	passVector.push_back(&sumLevelRaster);
	passVector.push_back(&sumLevelUpRaster);

	vector<raster *> getBackVector;
	getBackVector.push_back(&levelResultsToUseRaster);
	getBackVector.push_back(&zonesToCalibrateRaster);

	multipleRasterArithmetics(&getCalibratedZones, passVector, getBackVector);
	levelResultsToUseRaster.zonalStatistics(statisticsLevelUpRaster, levelSumToUseRaster, raster::SUM);
	statisticsLevelUpRaster.rasterArithmetics(&xminus, levelSumToUseRaster, differenceRaster);
	differenceRaster.rasterArithmetics(&selectAreaByMask, zonesToCalibrateRaster, calibratedStatisticsLevelRaster);

	validateCropland(inCroplandRaster,
		calibratedStatisticsLevelRaster,
		inClassRaster,
		resultsForCalibratedZonesRaster,
		outMinClassRaster);

	passVector.clear();
	passVector.push_back(&zonesToCalibrateRaster);
	passVector.push_back(&resultsForCalibratedZonesRaster);
	passVector.push_back(&resultLevelRaster);

	getBackVector.clear();
	getBackVector.push_back(&outCalibratedRasterLevel);

	multipleRasterArithmetics(&combineLevels, passVector, getBackVector);

	printf(__TIME__ "\n");
}

void getValidatedResults(const vector<float> & valVector,
						 const vector<float> & noDataValuesVector,
						 const vector<float> & noDataValuesOutVector,
						 vector<float> & result)
{
	float stat = valVector[0];
	float computed = valVector[1];

	if (compare_eq(stat, noDataValuesVector[0], EPSILON) || compare_eq(computed, noDataValuesVector[1], EPSILON))
	{
		result[0] = noDataValuesOutVector[0];
		result[1] = noDataValuesOutVector[1];
	}
	else
	{
		result[0] = fabs(stat - computed);
		result[1] = (stat != 0) ? result[0] / stat * (float)100 : (float)0;
	}
}

void validateResult(raster & cellAreaStatRaster,
					raster & statisticsRaster,
					raster & outAbsDiffRaster,
					raster & outRelDiffRaster,
					const runParamsT & runParams)
{
	//
	raster tmpResultingAreasRaster(runParams.tmpDir + "tmp_resulting_areas", raster::TEMPORARY);

	cellAreaStatRaster.zonalStatistics(statisticsRaster, tmpResultingAreasRaster, raster::SUM);

	vector<raster *> passVector;
	passVector.push_back(&statisticsRaster);
	passVector.push_back(&tmpResultingAreasRaster);

	vector<raster *> getBackVector;
	getBackVector.push_back(&outAbsDiffRaster);
	getBackVector.push_back(&outRelDiffRaster);

	multipleRasterArithmetics(&getValidatedResults, passVector, getBackVector);	
}