#include <iostream>
#include <vector>
#include <string>

#include "raster.h"
#include "commonTools.h"
#include "timer.h"

//#include "gisToolsInterface.h"
#include "utils.h"

void processListOfRasters(const vector<float> & croplandVector,
						  const vector<float> & croplandNoDataVector,
						  const vector<float> & noDataOutVector,
						  vector<float> & result,
						  void * priorityDataMap)
{
	size_t numRasters = (croplandVector.size() - 1) / 2;

	int countData = 0;
	float sumData = (float)0;
	int sumPowers = 0;
	int sumIsDataPowers = 0;
	float percentMin = (float)100;
	float percentMax = (float)0;

	int order = 1;
	bool isData = false;

	for (size_t idx = 0; idx < numRasters; idx++)
	{
		if ((croplandVector[idx] != croplandNoDataVector[idx]) && (croplandVector[idx] > 0))
		{
			countData += 1;
			sumData += croplandVector[idx];
			percentMin = xmin(percentMin, croplandVector[idx]);
			percentMax = xmax(percentMax, croplandVector[idx]);
			sumPowers += order;
			isData = true;
		}
		if ((croplandVector[numRasters + idx] != croplandNoDataVector[numRasters + idx]) && (croplandVector[numRasters + idx] > 0))
		{
			sumIsDataPowers += order; 
		}
		order *= 2;
	}

	if (isData)
	{
		int countryId = (int)croplandVector[2 * numRasters];
		map<int, priorityDataT * > * priorityDataMapPtr = ((map<int, priorityDataT * > *)priorityDataMap);
		map<int, priorityDataT * >::iterator countryPriorityData = priorityDataMapPtr->find(countryId);

		// If the country ID isn't found in list, use unit priorities for all rasters
		if (countryPriorityData == priorityDataMapPtr->end())
		{
			countryPriorityData = priorityDataMapPtr->find(NO_VALIDATION_POINTS_COUNTRY_ID);
		}
		priorityDataT * priorityData = countryPriorityData->second;

		float factor = ((float)(priorityData->agTable->getClass(sumIsDataPowers)) != (float)0)
			? (float)order / (float)(priorityData->agTable->getClass(sumIsDataPowers))
			: (float)1;
		result[0] = sumData / countData;				// avg
		result[1] = percentMin;							// min avg
		result[2] = (percentMin + result[0]) / 2;		// min
		result[3] = percentMax;							// max avg
		result[4] = (percentMax + result[0]) / 2;		// max
		result[5] = floor((float)(priorityData->agTable->getClass(sumPowers)) * factor);	// probability
		// Show areas where an overshooting was
		if (result[5] > order)
		{
			result[5] = (float)order;
			result[6] = (float)1;
		}
		else
		{
			result[6] = noDataOutVector[6];
		}
	}
	else
	{
		result[0] = noDataOutVector[0];
		result[1] = noDataOutVector[1];
		result[2] = noDataOutVector[2];
		result[3] = noDataOutVector[3];
		result[4] = noDataOutVector[4];
		result[5] = noDataOutVector[5];
		result[6] = noDataOutVector[6];
	}
}

int main(int argc, char * argv[])
{
	// Command line arguments:
    // 1 - workingDir
    // 2 - resultDir
    // 3 - tmpDir
    // 4 - cellAreas
    // 5 - countries
    // 6 - numRasters
    // 7.. - list of cropland layers
    // 8.. - csv file
    // 9.. - resulting probability classes raster
	// 10.. - resulting average raster
	// 11.. - resulting minimum raster
	// 12.. - resulting minimum average raster
	// 13.. - resulting maximum raster
	// 14.. - resulting maximum average raster
	// 15.. - selection threshold

	printf("Start: ");
	outputLocalTime();
	Timer timer;
	timer.start();

	runParamsT runParams;
	runParams.workingDir = string(argv[1]) + "\\";
	runParams.resultDir = string(argv[2]) + "\\";
	runParams.tmpDir = string(argv[3]) + "\\";

	raster areaRaster(argv[4], raster::INPUT);
	raster countriesRaster(argv[5], raster::INPUT);

	int numRasters = atoi(argv[6]);
	ASSERT_INT(argc == (15 + numRasters), INCORRECT_INPUT_PARAMS);

	size_t startListOfRasters = 7;

	string csvPriorityFile = argv[startListOfRasters + numRasters];
	printf("%s\n", csvPriorityFile.c_str());
	size_t startResults = startListOfRasters + numRasters + 1;

	// Resulting rasters
	raster resultProb(argv[startResults+0], raster::OUTPUT);
	raster resultAvg(argv[startResults+1], raster::OUTPUT);
	raster resultMin(argv[startResults+2], raster::OUTPUT);
	raster resultMinAvg(argv[startResults+3], raster::OUTPUT);
	raster resultMax(argv[startResults+4], raster::OUTPUT);
	raster resultMaxAvg(argv[startResults+5], raster::OUTPUT);
	raster resultOverflow((runParams.resultDir + "warning_overflow.img").c_str(), raster::OUTPUT);

	float selectionThreshold = (float)atof(argv[startResults + 6]);

	// Vectors for list of cropland rasters
	vector<raster *> croplandRastersVector;
	croplandRastersVector.resize(2 * numRasters + 1); // first half - cropland rasters; second half - cropland is data rasters;
													  // last one - countries raster

	// Initialize map of priorityData objects for each country
	map<int, priorityDataT * > priorityDataMap;
	initializePriorityData(csvPriorityFile, numRasters, priorityDataMap);

	// Initialize vector of resulting rasters
	vector<raster *> getBackVector;
	getBackVector.resize(7);
	getBackVector[0] = &resultAvg;
	getBackVector[1] = &resultMinAvg;
	getBackVector[2] = &resultMin;
	getBackVector[3] = &resultMaxAvg;
	getBackVector[4] = &resultMax;
	getBackVector[5] = &resultProb;
	getBackVector[6] = &resultOverflow;

	// Initializing vector of cropland rasters and related vectors
	for (size_t idx = 0; idx < (size_t)numRasters; idx++)
	{
		croplandRastersVector[idx] = new raster[1];
		*(croplandRastersVector[idx]) = raster(argv[startListOfRasters + idx], raster::PASS_INPUT);
		string tmpName = string(argv[startListOfRasters + idx]) + "_is_data";
		croplandRastersVector[numRasters + idx] = new raster[1];
		*(croplandRastersVector[numRasters + idx]) = raster(tmpName, raster::PASS_TEMPORARY);
		getAreasWithCropland(countriesRaster,
			*(croplandRastersVector[idx]),
			*(croplandRastersVector[numRasters + idx]),
			selectionThreshold,
			runParams);
	}
	croplandRastersVector[2 * numRasters] = &countriesRaster;

	multipleRasterArithmetics(&processListOfRasters, croplandRastersVector, getBackVector, (void *)&priorityDataMap);

	raster outClassRaster((runParams.resultDir + "adjusted_probability"), raster::OUTPUT);
	adjustCroplandProbabilityLayer(areaRaster,
		countriesRaster,
		*(getBackVector[5]),
		outClassRaster,
		runParams,
		priorityDataMap
		);

	// Destroy map of priorityData for counries
	destroyPriorityData(priorityDataMap);

	printf("End: ");
	outputLocalTime();
	timer.stop();
	printf("Elapsed time: %5.2f seconds.\n", timer.elapsedSeconds());

	return 0;
}