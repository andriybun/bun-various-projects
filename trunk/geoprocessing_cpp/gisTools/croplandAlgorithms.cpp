//#include <Python.h>
#include "raster.h"

void raster::zonalSumByClassAsTable(const raster & inZoneRaster,
									raster & inClassRaster,
									summaryTableT & calibratedResults)
{
	printf("Executing zonal sum by class (as table)\n");

	validateExtent(inZoneRaster);
	validateExtent(inClassRaster);
	ASSERT_INT(calibratedResults.size() == 0, OTHER_ERROR);

	string thisHdrPath = (*this).rasterPath + ".hdr";
	string thisFltPath = (*this).rasterPath + ".flt";
	string inZoneHdrPath = inZoneRaster.rasterPath + ".hdr";
	string inZoneFltPath = inZoneRaster.rasterPath + ".flt";
	string inClassHdrPath = inClassRaster.rasterPath + ".hdr";
	string inClassFltPath = inClassRaster.rasterPath + ".flt";

	// First run: computing statistics for probability raster
	statisticsStructT probabilityStatistics = inClassRaster.describe();
	size_t maxClass = (size_t)probabilityStatistics.maxVal;
	size_t minClass = (size_t)probabilityStatistics.minVal;
	tableT outTable;
	outTable.setNumCols(maxClass);
	printf("\tmin class: %d\n", minClass);
	printf("\tmax class: %d\n", maxClass);

	ifstream thisFile;
	thisFile.open(thisFltPath.c_str(), ios::in | ios::binary);
	ASSERT_INT(thisFile.is_open(), FILE_NOT_OPEN);
	ifstream inZoneFile;
	inZoneFile.open(inZoneFltPath.c_str(), ios::out | ios::binary);
	ASSERT_INT(inZoneFile.is_open(), FILE_NOT_OPEN);
	ifstream inClassFile;
	inClassFile.open(inClassFltPath.c_str(), ios::out | ios::binary);
	ASSERT_INT(inClassFile.is_open(), FILE_NOT_OPEN);

	int numCells = (*this).horResolution * (*this).verResolution;
	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);

	float * bufArea = new float[bufSize];
	float * bufZone = new float[bufSize];
	float * bufClass = new float[bufSize];

	int numCellsProcessed = 0;

	// Second run: compute statistics for classes per country
	printf("Computing statistics for classes per country\n");
	numCellsProcessed = 0;
	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		numCellsProcessed += bufSize;
		thisFile.read(reinterpret_cast<char*>(bufArea), sizeof(float) * bufSize);
		inZoneFile.read(reinterpret_cast<char*>(bufZone), sizeof(float) * bufSize);
		inClassFile.read(reinterpret_cast<char*>(bufClass), sizeof(float) * bufSize);
		for (int i = 0; i < bufSize; i++)
		{
			if ((bufArea[i] != (*this).noDataValue) &&
				(bufZone[i] != inZoneRaster.noDataValue) &&
				(bufClass[i] != inClassRaster.noDataValue))
			{
				outTable.inc(bufZone[i], (size_t)bufClass[i], bufArea[i]);
			}
		}
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}

	thisFile.close();
	inZoneFile.close();
	inClassFile.close();

	delete [] bufArea;
	delete [] bufZone;
	delete [] bufClass;

	// Processing table with computed on previous step statistics
	printf("Analyzing collected statistics\n");

	tableT::dataT::iterator row = outTable.data.begin();
	size_t currentCountry = 0;
	printf("Zone ID\tZone stats\tBest estimate\tBest class\tError\n");
	while (row != outTable.data.end())
	{
		float targetSum = row->first;
		//cout << "target sum = " << targetSum << endl;
		float rowSum = (float)0;
		float resultingSum = (float)0;
		float absDiff = targetSum;
		unitResultT rowResult;
		rowResult.bestClass = -1;
		rowResult.bestEstimate = rowSum;
		//cout << maxClass-1 << "\t" << minClass-1 << endl;
		for (int cl = (int)maxClass; cl >= (int)minClass; cl--)
		{
			rowSum += row->second[cl-1];
			float curDiff = fabs(targetSum - rowSum);
			if ((curDiff <= absDiff) || (rowResult.bestEstimate == (float)0))
			{
				absDiff = curDiff;
				rowResult.bestEstimate = rowSum;
				rowResult.bestClass = cl;
			}
		}
		rowResult.error = absDiff;
		printf("%d\t%f\t%f\t%d\t\t%f\n", currentCountry, targetSum, rowResult.bestEstimate, rowResult.bestClass, rowResult.error);
		calibratedResults.insert(make_pair<float, unitResultT>(row->first, rowResult));
		currentCountry++;
		row++;
	}
}

void validateCropland(raster & inCroplandRaster,
					  raster & inZoneRaster,
					  raster & inClassRaster,
					  raster & outCroplandRaster,
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
	string outErrorHdrPath = outErrorRaster.rasterPath + ".hdr";
	string outErrorFltPath = outErrorRaster.rasterPath + ".flt";

	inCroplandRaster.copyFile(inZoneHdrPath, outCroplandHdrPath);
	inCroplandRaster.copyFile(inZoneHdrPath, outErrorHdrPath);
	inCroplandRaster.copyProperties(outCroplandRaster);
	inCroplandRaster.copyProperties(outErrorRaster);

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

	int numCells = inCroplandRaster.horResolution * inCroplandRaster.verResolution;
	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);

	float * inBufCropland = new float[bufSize];
	float * inBufZone = new float[bufSize];
	float * inBufClass = new float[bufSize];
	float * outBufCropland = new float[bufSize];
	float * outBufError = new float[bufSize];

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
					}
					else
					{
						outBufError[i] = outErrorRaster.noDataValue;
					}
				}
				else
				{
					outBufError[i] = outErrorRaster.noDataValue;
				}
				outBufCropland[i] = outCroplandRaster.noDataValue;
			}
		}
		outCroplandFile.write(reinterpret_cast<char *>(outBufCropland), sizeof(float) * bufSize);
		outErrorFile.write(reinterpret_cast<char *>(outBufError), sizeof(float) * bufSize);
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}

	inCroplandFile.close();
	inZoneFile.close();
	inClassFile.close();
	outCroplandFile.close();
	outErrorFile.close();

	delete [] inBufCropland;
	delete [] inBufZone;
	delete [] inBufClass;
	delete [] outBufCropland;
	delete [] outBufError;
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
	validateCropland(inCroplandRaster, calibratedStatisticsLevelRaster, inClassRaster, resultsForCalibratedZonesRaster);

	passVector.clear();
	passVector.push_back(&zonesToCalibrateRaster);
	passVector.push_back(&resultsForCalibratedZonesRaster);
	passVector.push_back(&resultLevelRaster);

	getBackVector.clear();
	getBackVector.push_back(&outCalibratedRasterLevel);

	multipleRasterArithmetics(&combineLevels, passVector, getBackVector);

	//levelResultsToUseRaster.convertFloatToRaster();
	//levelSumToUseRaster.convertFloatToRaster();
	//zonesToCalibrateRaster.convertFloatToRaster();
	//differenceRaster.convertFloatToRaster();
	//resultsForCalibratedZonesRaster.convertFloatToRaster();
	//calibratedStatisticsLevelRaster.convertFloatToRaster();

	printf(__TIME__ "\n");
}