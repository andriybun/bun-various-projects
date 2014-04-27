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

	map< int, float * > classesByCountryTable;	// key - country ID, value - vector of areas for all classes
	map< int, vector<int> > oldToNewClassesMap;	// key - country ID, value - vector of new (adjusted) classes

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
				map< int, float * >::iterator it = classesByCountryTable.find((int)bufCountries[i]);
				if (it == classesByCountryTable.end())
				{
					printf("  > adding: c = %d\n", (int)bufCountries[i]);
					float * tmp = new float[maxClass + 1];
					memset(tmp, 0, sizeof(float) * (maxClass + 1));
					classesByCountryTable.insert(make_pair<int, float *>((int)bufCountries[i], tmp));
					oldToNewClassesMap.insert(make_pair<int, vector<int> >((int)bufCountries[i], dummyVector));
					it = classesByCountryTable.find((int)bufCountries[i]);
				}
				it->second[(int)bufClass[i]] += bufArea[i];
			}
		}
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}
	
	// Second run through the table - rearrange classes
	map< int, float * >::iterator it = classesByCountryTable.begin();
	while (it != classesByCountryTable.end())
	{
		for (size_t i = 0; i <= maxClass; i++)
		{
			oldToNewClassesMap[it->first][i] = -1;
		}

		for (size_t i = minClass; i <= maxClass; i++)
		{
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

		}
	
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
				outBufClass[i] = (float)oldToNewClassesMap[(int)bufCountries[i]][(int)bufClass[i]];
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

void validateCropland(
					  // Input cropland raster containing information about the
					  // percentage of cropland per cell
					  raster & inCroplandRawRaster,
					  // Input cropland raster containing information about the
					  // physical area of cropland per cell
					  raster & inCroplandRaster,
					  // Input zone raster defining zones (countries, regions etc.).
					  // Here all cells with the same value are considered as a zone
					  raster & inZoneRaster,
					  // Input raster defining to which probability class each cell
					  // belongs
					  raster & inClassRaster,
					  // Resulting raster
					  raster & outCroplandRaster,
					  raster & outMinClassRaster,
					  const runParamsT & runParams)
{
	// Compute statistics
	raster::summaryTableT calibratedResults;
	inCroplandRaster.zonalSumByClassAsTable(inZoneRaster, inClassRaster, calibratedResults, runParams);

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

	ifstream inCroplandRawFile;
	inCroplandRawFile.open(inCroplandRawRaster.getFltPath().c_str(), ios::out | ios::binary);
	ifstream inZoneFile;
	inZoneFile.open(inZoneFltPath.c_str(), ios::out | ios::binary);
	ifstream inClassFile;
	inClassFile.open(inClassFltPath.c_str(), ios::out | ios::binary);
	ofstream outCroplandFile;
	outCroplandFile.open(outCroplandFltPath.c_str(), ios::out | ios::binary);
	ofstream outMinClassFile;
	outMinClassFile.open(outMinClassFltPath.c_str(), ios::out | ios::binary);

	int numCells = inCroplandRawRaster.horResolution * inCroplandRawRaster.verResolution;
	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);

	float * inBufCroplandRaw = new float[bufSize];
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
		inCroplandRawFile.read(reinterpret_cast<char*>(inBufCroplandRaw), sizeof(float) * bufSize);
		inZoneFile.read(reinterpret_cast<char*>(inBufZone), sizeof(float) * bufSize);
		inClassFile.read(reinterpret_cast<char*>(inBufClass), sizeof(float) * bufSize);
		for (int i = 0; i < bufSize; i++)
		{
			if ((inBufCroplandRaw[i] != inCroplandRaster.noDataValue)
				&& (inBufZone[i] != inZoneRaster.noDataValue)
				&& (inBufClass[i] != inClassRaster.noDataValue))
			{
				raster::unitResultT unitResult = calibratedResults[(int)inBufZone[i]];
				outBufMinClass[i] = (float)unitResult.bestClass;
				if (unitResult.bestClass < inBufClass[i])
				{
					outBufCropland[i] = inBufCroplandRaw[i];
				}
				else if (unitResult.bestClass == inBufClass[i])
				{
					outBufCropland[i] = inBufCroplandRaw[i] * unitResult.bestClassMultiplier;
				}
				else
				{
					outBufCropland[i] = outCroplandRaster.noDataValue;
				}
			}
			else
			{
				if (inBufZone[i] != inZoneRaster.noDataValue)
				{
					raster::summaryTableT::iterator unitResultIter = calibratedResults.find((int)inBufZone[i]);
					if (unitResultIter != calibratedResults.end())
					{
						raster::unitResultT unitResult = unitResultIter->second;
						outBufMinClass[i] = (float)unitResult.bestClass;
					}
				}
				outBufCropland[i] = outCroplandRaster.noDataValue;
			}
		}
		outCroplandFile.write(reinterpret_cast<char *>(outBufCropland), sizeof(float) * bufSize);
		outMinClassFile.write(reinterpret_cast<char *>(outBufMinClass), sizeof(float) * bufSize);
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}

	inCroplandRawFile.close();
	inZoneFile.close();
	inClassFile.close();
	outCroplandFile.close();
	outMinClassFile.close();

	delete [] inBufCroplandRaw;
	delete [] inBufZone;
	delete [] inBufClass;
	delete [] outBufCropland;
	delete [] outBufMinClass;
}

//void validateCropland(
//					  // Input cropland raster containing information about the
//					  // percentage of cropland per cell
//					  raster & inCroplandRawRaster,
//					  // Input cropland raster containing information about the
//					  // physical area of cropland per cell
//					  raster & inCroplandRaster,
//					  // Input zone raster defining zones (countries, regions etc.).
//					  // Here all cells with the same value are considered as a zone
//					  raster & inZoneRaster,
//					  // Input raster defining to which probability class each cell
//					  // belongs
//					  raster & inClassRaster,
//					  // Resulting raster
//					  raster & outCroplandRaster,
//					  raster & outMinClassRaster,
//					  raster & outErrorRaster,
//					  const runParamsT & runParams)
//{
	//// Compute statistics
	//raster::summaryTableT calibratedResults;
	//inCroplandRaster.zonalSumByClassAsTable(inZoneRaster, inClassRaster, calibratedResults, runParams);

	//// Write statistics to resulting files
	//string inCroplandHdrPath = inCroplandRaster.rasterPath + ".hdr";
	//string inCroplandFltPath = inCroplandRaster.rasterPath + ".flt";
	//string inZoneHdrPath = inZoneRaster.rasterPath + ".hdr";
	//string inZoneFltPath = inZoneRaster.rasterPath + ".flt";
	//string inClassFltPath = inClassRaster.rasterPath + ".flt";
	//string inClassHdrPath = inClassRaster.rasterPath + ".hdr";
	//string outCroplandHdrPath = outCroplandRaster.rasterPath + ".hdr";
	//string outCroplandFltPath = outCroplandRaster.rasterPath + ".flt";
	//string outMinClassHdrPath = outMinClassRaster.rasterPath + ".hdr";
	//string outMinClassFltPath = outMinClassRaster.rasterPath + ".flt";

	//inCroplandRaster.copyFile(inZoneHdrPath, outCroplandHdrPath);
	//inCroplandRaster.copyFile(inZoneHdrPath, outMinClassHdrPath);
	//inCroplandRaster.copyProperties(outCroplandRaster);
	//inCroplandRaster.copyProperties(outMinClassRaster);

	//ifstream inCroplandRawFile;
	//inCroplandRawFile.open(inCroplandRawRaster.getFltPath().c_str(), ios::out | ios::binary);
	//ifstream inZoneFile;
	//inZoneFile.open(inZoneFltPath.c_str(), ios::out | ios::binary);
	//ifstream inClassFile;
	//inClassFile.open(inClassFltPath.c_str(), ios::out | ios::binary);
	//ofstream outCroplandFile;
	//outCroplandFile.open(outCroplandFltPath.c_str(), ios::out | ios::binary);
	//ofstream outMinClassFile;
	//outMinClassFile.open(outMinClassFltPath.c_str(), ios::out | ios::binary);

	//int numCells = inCroplandRawRaster.horResolution * inCroplandRawRaster.verResolution;
	//int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);

	//float * inBufCroplandRaw = new float[bufSize];
	//float * inBufZone = new float[bufSize];
	//float * inBufClass = new float[bufSize];
	//float * outBufCropland = new float[bufSize];
	//float * outBufMinClass = new float[bufSize];

	//printf("Writing results to files\n");
	//int numCellsProcessed = 0;
	//while(numCellsProcessed < numCells)
	//{
	//	bufSize = min(bufSize, numCells - numCellsProcessed);
	//	numCellsProcessed += bufSize;
	//	inCroplandRawFile.read(reinterpret_cast<char*>(inBufCroplandRaw), sizeof(float) * bufSize);
	//	inZoneFile.read(reinterpret_cast<char*>(inBufZone), sizeof(float) * bufSize);
	//	inClassFile.read(reinterpret_cast<char*>(inBufClass), sizeof(float) * bufSize);
	//	for (int i = 0; i < bufSize; i++)
	//	{
	//		if ((inBufCroplandRaw[i] != inCroplandRaster.noDataValue)
	//			&& (inBufZone[i] != inZoneRaster.noDataValue)
	//			&& (inBufClass[i] != inClassRaster.noDataValue))
	//		{
	//			raster::unitResultT unitResult = calibratedResults[(int)inBufZone[i]];
	//			outBufMinClass[i] = (float)unitResult.bestClass;
	//			if (unitResult.bestClass < inBufClass[i])
	//			{
	//				outBufCropland[i] = inBufCroplandRaw[i];
	//			}
	//			else if (unitResult.bestClass == inBufClass[i])
	//			{
	//				outBufCropland[i] = inBufCroplandRaw[i] * unitResult.bestClassMultiplier;
	//			}
	//			else
	//			{
	//				outBufCropland[i] = outCroplandRaster.noDataValue;
	//			}
	//		}
	//		else
	//		{
	//			if (inBufZone[i] != inZoneRaster.noDataValue)
	//			{
	//				raster::summaryTableT::iterator unitResultIter = calibratedResults.find((int)inBufZone[i]);
	//				if (unitResultIter != calibratedResults.end())
	//				{
	//					raster::unitResultT unitResult = unitResultIter->second;
	//					outBufMinClass[i] = (float)unitResult.bestClass;
	//				}
	//			}
	//			outBufCropland[i] = outCroplandRaster.noDataValue;
	//		}
	//	}
	//	outCroplandFile.write(reinterpret_cast<char *>(outBufCropland), sizeof(float) * bufSize);
	//	outMinClassFile.write(reinterpret_cast<char *>(outBufMinClass), sizeof(float) * bufSize);
	//	printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	//}

	//inCroplandRawFile.close();
	//inZoneFile.close();
	//inClassFile.close();
	//outCroplandFile.close();
	//outMinClassFile.close();

	//delete [] inBufCroplandRaw;
	//delete [] inBufZone;
	//delete [] inBufClass;
	//delete [] outBufCropland;
	//delete [] outBufMinClass;
//}

void validateCropland(
					  // Input cropland raster containing information about the
					  // percentage of cropland per cell
					  raster & inCroplandRawRaster,
					  // Input cropland raster containing information about the
					  // physical area of cropland per cell
					  raster & inCroplandRaster,
					  // Input zone raster defining zones (countries, regions etc.).
					  // Here all cells with the same value are considered as a zone
					  raster & inZoneRaster,
					  // Input raster defining to which probability class each cell
					  // belongs
					  raster & inClassRaster,
					  // Resulting raster
					  raster & outCroplandRaster,
					  const runParamsT & runParams)
{
	// Compute statistics
	raster::summaryTableT calibratedResults;
	inCroplandRaster.zonalSumByClassAsTable(inZoneRaster, inClassRaster, calibratedResults, runParams);

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

	ifstream inCroplandRawFile;
	inCroplandRawFile.open(inCroplandRawRaster.getFltPath().c_str(), ios::out | ios::binary);
	ifstream inZoneFile;
	inZoneFile.open(inZoneFltPath.c_str(), ios::out | ios::binary);
	ifstream inClassFile;
	inClassFile.open(inClassFltPath.c_str(), ios::out | ios::binary);
	ofstream outCroplandFile;
	outCroplandFile.open(outCroplandFltPath.c_str(), ios::out | ios::binary);

	int numCells = inCroplandRawRaster.horResolution * inCroplandRawRaster.verResolution;
	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);

	float * inBufCroplandRaw = new float[bufSize];
	float * inBufZone = new float[bufSize];
	float * inBufClass = new float[bufSize];
	float * outBufCropland = new float[bufSize];

	printf("Writing results to files\n");
	int numCellsProcessed = 0;
	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		numCellsProcessed += bufSize;
		inCroplandRawFile.read(reinterpret_cast<char*>(inBufCroplandRaw), sizeof(float) * bufSize);
		inZoneFile.read(reinterpret_cast<char*>(inBufZone), sizeof(float) * bufSize);
		inClassFile.read(reinterpret_cast<char*>(inBufClass), sizeof(float) * bufSize);
		for (int i = 0; i < bufSize; i++)
		{
			if ((inBufCroplandRaw[i] != inCroplandRawRaster.noDataValue)
				&& (inBufZone[i] != inZoneRaster.noDataValue)
				&& (inBufClass[i] != inClassRaster.noDataValue))
			{
				raster::unitResultT unitResult = calibratedResults[(int)inBufZone[i]];
				if (unitResult.bestClass < inBufClass[i])
				{
					outBufCropland[i] = inBufCroplandRaw[i];
				}
				else if (unitResult.bestClass == inBufClass[i])
				{
					outBufCropland[i] = inBufCroplandRaw[i] * unitResult.bestClassMultiplier;
				}
				else
				{
					outBufCropland[i] = outCroplandRaster.noDataValue;
				}
			}
			else
			{
				if (inBufZone[i] != inZoneRaster.noDataValue)
				{
					raster::summaryTableT::iterator unitResultIter = calibratedResults.find((int)inBufZone[i]);
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

	inCroplandRawFile.close();
	inZoneFile.close();
	inClassFile.close();
	outCroplandFile.close();

	delete [] inBufCroplandRaw;
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
		result[0] = ((sumLevel > sumLevelUp) && (resultLevel != noDataValuesVector[0])) ? resultLevel : noDataValuesOutVector[0];
		// get mask of cells for which calibration is needed
		result[1] = (sumLevel <= sumLevelUp) ? 1 : noDataValuesOutVector[1];
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

void calibrateCropland(raster & inCroplandRawRaster,
					   raster & inCroplandRaster,
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

	validateCropland(
		inCroplandRawRaster,
		inCroplandRaster,
		calibratedStatisticsLevelRaster,
		inClassRaster,
		resultsForCalibratedZonesRaster,
		runParams);

	passVector.clear();
	passVector.push_back(&zonesToCalibrateRaster);
	passVector.push_back(&resultsForCalibratedZonesRaster);
	passVector.push_back(&resultLevelRaster);

	getBackVector.clear();
	getBackVector.push_back(&outCalibratedRasterLevel);

	multipleRasterArithmetics(&combineLevels, passVector, getBackVector);

	printf(__TIME__ "\n");
}

void calibrateCropland(raster & inCroplandRawRaster,
					   raster & inCroplandRaster,
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

	validateCropland(
		inCroplandRawRaster,
		inCroplandRaster,
		calibratedStatisticsLevelRaster,
		inClassRaster,
		resultsForCalibratedZonesRaster,
		outMinClassRaster,
		runParams);

	passVector.clear();
	passVector.push_back(&zonesToCalibrateRaster);
	passVector.push_back(&resultsForCalibratedZonesRaster);
	passVector.push_back(&resultLevelRaster);

	getBackVector.clear();
	getBackVector.push_back(&outCalibratedRasterLevel);

	multipleRasterArithmetics(&combineLevels, passVector, getBackVector);

	printf(__TIME__ "\n");
}

float preprocessCellAreasInt(float area, float stat)
{
	return area * stat / (float)1000;
}

void getValidatedResults(const vector<float> & valVector,
						 const vector<float> & noDataValuesVector,
						 const vector<float> & noDataValuesOutVector,
						 vector<float> & result)
{
	float stat = valVector[0];
	float computed = valVector[1];
	float resCropland = valVector[2];

	if (compare_eq(stat, noDataValuesVector[0], EPSILON) 
		|| compare_eq(computed, noDataValuesVector[1], EPSILON))
	{
		result[0] = noDataValuesOutVector[0];
		result[1] = noDataValuesOutVector[1];
		result[2] = noDataValuesOutVector[2];
		result[3] = noDataValuesOutVector[3];
	}
	else
	{
		result[0] = fabs(stat - computed);
		result[1] = (stat != 0) ? result[0] / stat * (float)100 : (float)0;
		result[2] = ((computed < stat) && (result[1] > 0.02)) ? (float)1: (float)0;
		if (compare_eq(resCropland, noDataValuesVector[0], EPSILON))
		{
			result[3] = noDataValuesOutVector[3];
		}
		else 
		{
			result[3] = (computed < stat) ? xmin(resCropland * stat / computed, (float)100) : resCropland;
		}
	}
}

void validateResult(raster & areaRaster,
					raster & computedResultRaster,
					raster & statisticsRaster,
					raster & outTotalCroplandRaster,
					raster & outAbsDiffRaster,
					raster & outRelDiffRaster,
					raster & outAdjustedResultRaster,
					const runParamsT & runParams)
{
	raster tmpComputedAreasRaster(runParams.tmpDir + "tmp_computed_areas", raster::TEMPORARY);
	raster tmpRatioRaster(runParams.tmpDir + "tmp_ratio", raster::TEMPORARY);
	raster debugNotEnoughRaster(runParams.debugDir + computedResultRaster.getName() + "debug_not_enough", raster::OUTPUT);

	// Do some arithmetic transformations of input rasters
	areaRaster.rasterArithmetics(&preprocessCellAreasInt, computedResultRaster, tmpComputedAreasRaster);
	// Calculate zonal sum per countries (specified zones)
	tmpComputedAreasRaster.zonalStatistics(statisticsRaster, outTotalCroplandRaster, raster::SUM);

	// Pack in/out vectors for validation function
	vector<raster *> passVector;
	passVector.push_back(&statisticsRaster);
	passVector.push_back(&outTotalCroplandRaster);
	passVector.push_back(&computedResultRaster);

	vector<raster *> getBackVector;
	getBackVector.push_back(&outAbsDiffRaster);
	getBackVector.push_back(&outRelDiffRaster);
	getBackVector.push_back(&debugNotEnoughRaster);
	getBackVector.push_back(&outAdjustedResultRaster);

	multipleRasterArithmetics(&getValidatedResults, passVector, getBackVector);	
}