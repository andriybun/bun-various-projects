#include <iostream>
#include <vector>
// additional include directory: $(PYTHON_INCLUDE)
//#include <Python.h>

#include "raster.h"
#include "agreementTable.h"
#include "commonTools.h"
#include "timer.h"

#include "gisToolsInterface.h"

const float selectionThreshold = (float)0.01;

#define GENERATE_RASTER_WITH_NAME_TEMPLATE(RASTER_NAME, NAME_TEMPLATE, ...)		\
	{																			\
		char tmpName[1000];														\
		sprintf_s(tmpName, NAME_TEMPLATE, __VA_ARGS__);							\
		string tmpNameStr = string(tmpName);									\
		RASTER_NAME = new raster[1];											\
		*(RASTER_NAME) = raster(tmpNameStr, raster::PASS_TEMPORARY);			\
	}

struct priorityDataT
{
	vector<int> prioritiesVector;
	vector<int> prioritiesVector2;
	vector<int> weightsVector;
	agreementTableT * agTable;
};

void processListOfRasters(const vector<float> & croplandVector,
						  const vector<float> & croplandNoDataVector,
						  const vector<float> & noDataOutVector,
						  vector<float> & result,
						  void * priorityData)
{
	size_t numRasters = croplandVector.size() / 2;

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

	result[0] = isData ? sumData / countData : noDataOutVector[0];		// avg
	result[1] = isData ? percentMin : noDataOutVector[1];				// min
	result[2] = isData ? percentMax : noDataOutVector[2];				// max
	result[3] = isData																	// probability
		? floor(
			(float)(((priorityDataT *)priorityData)->agTable->getClass(sumPowers)) /
			(float)(((priorityDataT *)priorityData)->agTable->getClass(sumIsDataPowers)) * 
			100)
		: noDataOutVector[3];
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
	// 13.. - resulting minimum raster
	// 14.. - resulting maximum raster

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
	
	ASSERT_INT(argc == (11 + numRasters * 4), INCORRECT_INPUT_PARAMS);

	// Further arguments' offsets
	size_t startListOfRasters = 7 + 0 * numRasters;
	size_t startListOfPriorities = 7 + 1 * numRasters;
	size_t startListOfPriorities2 = 7 + 2 * numRasters;
	size_t startListOfWeights = 7 + 3 * numRasters;
	size_t startResults = 7 + 4 * numRasters;

	// Resulting rasters
	raster resultProb(argv[startResults+0], raster::OUTPUT);
	raster resultAvg(argv[startResults+1], raster::OUTPUT);
	raster resultMin(argv[startResults+2], raster::OUTPUT);
	raster resultMax(argv[startResults+3], raster::OUTPUT);

	// Vectors for list of cropland rasters
	vector<raster *> croplandRastersVector;
	croplandRastersVector.resize(2 * numRasters); // first half - cropland rasters; second half - cropland is data rasters;

	priorityDataT * priorityData = new priorityDataT[1];
	priorityData->prioritiesVector.resize(numRasters);
	priorityData->prioritiesVector2.resize(numRasters);
	priorityData->weightsVector.resize(numRasters);

	vector<raster *> getBackVector;
	getBackVector.resize(4);
	getBackVector[0] = &resultAvg;
	getBackVector[1] = &resultMin;
	getBackVector[2] = &resultMax;
	getBackVector[3] = &resultProb;

	// Initializing vector of cropland rasters and related vectors
	for (size_t idx = 0; idx < (size_t)numRasters; idx++)
	{
		croplandRastersVector[idx] = new raster[1];
		*(croplandRastersVector[idx]) = raster(argv[startListOfRasters + idx], raster::PASS_INPUT);
		priorityData->prioritiesVector[idx] = atoi(argv[startListOfPriorities + idx]);
		priorityData->prioritiesVector2[idx] = atoi(argv[startListOfPriorities2 + idx]);
		priorityData->weightsVector[idx] = atoi(argv[startListOfWeights + idx]);
		string tmpName = string(argv[startListOfRasters + idx]) + "_is_data";
		raster isDataRaster(tmpName, raster::OUTPUT);
		croplandRastersVector[numRasters + idx] = new raster[1];
		*(croplandRastersVector[numRasters + idx]) = raster(tmpName, raster::PASS_TEMPORARY);
		getAreasWithCropland(areaRaster,
			*(croplandRastersVector[idx]),
			*(croplandRastersVector[numRasters + idx]),
			selectionThreshold,
			runParams);
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