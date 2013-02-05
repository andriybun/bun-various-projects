#include <iostream>
#include <vector>
// additional include directory: $(PYTHON_INCLUDE)
//#include <Python.h>

#include "raster.h"
#include "agreementTable.h"
#include "commonTools.h"
#include "timer.h"

#define GENERATE_RASTER_WITH_NAME_TEMPLATE(RASTER_NAME, NAME_TEMPLATE, ...)		\
	{																			\
		char tmpName[1000];														\
		sprintf_s(tmpName, NAME_TEMPLATE, __VA_ARGS__);							\
		string tmpNameStr = string(tmpName);									\
		RASTER_NAME = new raster[1];											\
		*(RASTER_NAME) = raster(tmpNameStr, raster::PASS_TEMPORARY);			\
	}

void processListOfRasters(const vector<float> & croplandVector,
						  const vector<float> & croplandNoDataVector,
						  const vector<float> & noDataOutVector,
						  vector<float> & result,
						  void * priorityData)
{
	size_t numRasters = croplandVector.size();

	int countData = 0;
	float sumData = (float)0;
	int sumPowers = 0;
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
			sumPowers += (int)order;
			isData = true;
		}
		order *= 2;
	}

	result[0] = isData ? sumData / countData : noDataOutVector[0];			// avg
	result[2] = isData ? percentMin : noDataOutVector[2];					// min
	result[1] = isData ? (result[0] + result[2]) / 2 : noDataOutVector[1];	// min avg
	result[4] = isData ? percentMax : noDataOutVector[4];					// max
	result[3] = isData ? (result[0] + result[4]) / 2 : noDataOutVector[3];	// max avg
	result[5] = isData																	// probability
		? (float)(((priorityDataT *)priorityData)->agTable->getClass(sumPowers))
		: noDataOutVector[5];
}

int main(int argc, char * argv[])
{
	// Command line arguments:
	// 1 - workingDir
	// 2 - resultDir
	// 3 - tmpDir
	// 4 - cell areas
	// 5 - countries
	// 6 - number of rasters
	// 7.. - list of cropland layers
	// 8.. - list of priorities
	// 9.. - list of 2nd type priorities 
	// 10.. - list of rasters weights
	// 11.. - resulting probability classes raster
	// 12.. - resulting average raster
	// 13.. - resulting minimum average raster
	// 14.. - resulting minimum raster
	// 15.. - resulting maximum average raster
	// 16.. - resulting maximum raster

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
	
	ASSERT_INT(argc == (13 + numRasters * 4), INCORRECT_INPUT_PARAMS);

	// Further arguments' offsets
	size_t startListOfRasters = 7 + 0 * numRasters;
	size_t startListOfPriorities = 7 + 1 * numRasters;
	size_t startListOfPriorities2 = 7 + 2 * numRasters;
	size_t startListOfWeights = 7 + 3 * numRasters;
	size_t startResults = 7 + 4 * numRasters;

	// Resulting rasters
	raster resultProb(argv[startResults+0], raster::OUTPUT);
	raster resultAvg(argv[startResults+1], raster::OUTPUT);
	raster resultMinAvg(argv[startResults+2], raster::OUTPUT);
	raster resultMin(argv[startResults+3], raster::OUTPUT);
	raster resultMaxAvg(argv[startResults+4], raster::OUTPUT);
	raster resultMax(argv[startResults+5], raster::OUTPUT);

	// Vectors for list of cropland rasters
	vector<raster *> croplandRastersVector;
	priorityDataT * priorityData = new priorityDataT[1];
	
	croplandRastersVector.resize(numRasters);
	priorityData->prioritiesVector.resize(numRasters);
	priorityData->prioritiesVector2.resize(numRasters);
	priorityData->weightsVector.resize(numRasters);

	vector<raster *> getBackVector;
	getBackVector.resize(6);
	getBackVector[0] = &resultAvg;
	getBackVector[1] = &resultMinAvg;
	getBackVector[2] = &resultMin;
	getBackVector[3] = &resultMaxAvg;
	getBackVector[4] = &resultMax;
	getBackVector[5] = &resultProb;

	// Initializing vector of cropland rasters and related vectors
	for (size_t idx = 0; idx < (size_t)numRasters; idx++)
	{
		croplandRastersVector[idx] = new raster[1];
		*(croplandRastersVector[idx]) = raster(argv[startListOfRasters + idx], raster::PASS_INPUT);
		priorityData->prioritiesVector[idx] = atoi(argv[startListOfPriorities + idx]);
		priorityData->prioritiesVector2[idx] = atoi(argv[startListOfPriorities2 + idx]);
		priorityData->weightsVector[idx] = atoi(argv[startListOfWeights + idx]);
	}

	priorityData->agTable = new agreementTableT(priorityData->prioritiesVector, priorityData->prioritiesVector2);
	multipleRasterArithmetics(&processListOfRasters, croplandRastersVector, getBackVector, (void *)priorityData);

	// Free up memory
	delete priorityData->agTable;
	delete priorityData;

	for (size_t idx = 0; idx < (size_t)numRasters; idx++)
	{
		delete [] croplandRastersVector[idx];
	}

	printf("End: ");
	outputLocalTime();
	timer.stop();
	printf("Elapsed time: %5.2f seconds.\n", timer.elapsedSeconds());

	return 0;
}