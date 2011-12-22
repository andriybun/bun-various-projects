#include <iostream>
#include <vector>
// additional include directory: $(PYTHON_INCLUDE)
//#include <Python.h>

#include "raster.h"
#include "timer.h"

#define GENERATE_RASTER_WITH_NAME_TEMPLATE(RASTER_NAME, NAME_TEMPLATE, ...)		\
	{																			\
		char tmpName[1000];														\
		sprintf_s(tmpName, NAME_TEMPLATE, __VA_ARGS__);							\
		string tmpNameStr = string(tmpName);									\
		RASTER_NAME = new raster(tmpNameStr, raster::TEMPORARY);				\
	}

struct priorityDataT
{
	vector<int> prioritiesVector;
	vector<int> prioritiesVector2;
	vector<int> weightsVector;
};

void processListOfRasters(const vector<float> & croplandVector,
						  const vector<float> & croplandNoDataVector,
						  const vector<float> & noDataOutVector,
						  vector<float> & result,
						  void * priorityData)
{
	size_t numRasters = croplandVector.size();
	//size_t ordersVectorOffset = 0 * numRasters;
	//size_t weightsVectorOffset = 1 * numRasters;
	//size_t weights2VectorOffset = 2 * numRasters;

	int countData = 0;
	float sumData = (float)0;
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
			result[numRasters + 0 * idx] = (float)order;
			result[numRasters + 1 * idx] = (float)(((priorityDataT *)priorityData)->weightsVector[idx]);
			result[numRasters + 2 * idx] = (float)(((priorityDataT *)priorityData)->prioritiesVector2[idx]);
			isData = true;
		}
		else
		{
			result[numRasters + 0 * idx] = (float)0;
			result[numRasters + 1 * idx] = (float)0;
			result[numRasters + 2 * idx] = (float)0;

		}
		order *= 2;
	}

	result[numRasters * 3 + 0] = isData ? sumData / countData : noDataOutVector[numRasters * 3 + 0];	// avg
	result[numRasters * 3 + 1] = isData ? percentMin : noDataOutVector[numRasters * 3 + 1];				// min
	result[numRasters * 3 + 2] = isData ? percentMax : noDataOutVector[numRasters * 3 + 2];				// max
}

int main(int argc, char * argv[])
{
	//ASSERT_INT(argc == 11, INCORRECT_INPUT_PARAMS);

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
	priorityDataT * priorityData = new priorityDataT;
	
	croplandRastersVector.resize(numRasters);
	priorityData->prioritiesVector.resize(numRasters);
	priorityData->prioritiesVector2.resize(numRasters);
	priorityData->weightsVector.resize(numRasters);

	// Temporary raster vectors
	vector<raster *> croplandOrdersVector;
	vector<raster *> croplandWeightsVector;
	vector<raster *> croplandWeightsVector2;

	croplandOrdersVector.resize(numRasters);
	croplandWeightsVector.resize(numRasters);
	croplandWeightsVector2.resize(numRasters);

	vector<raster *> getBackVector;
	getBackVector.resize(numRasters * 3 + 3);
	getBackVector[numRasters * 3 + 0] = &resultAvg;
	getBackVector[numRasters * 3 + 1] = &resultMin;
	getBackVector[numRasters * 3 + 2] = &resultMax;

	// Initializing vector of cropland rasters and related vectors
	for (size_t idx = 0; idx < (size_t)numRasters; idx++)
	{
		croplandRastersVector[idx] = new raster(argv[startListOfRasters + idx], raster::INPUT);
		priorityData->prioritiesVector[idx] = atoi(argv[startListOfPriorities + idx]);
		priorityData->prioritiesVector2[idx] = atoi(argv[startListOfPriorities2 + idx]);
		priorityData->weightsVector[idx] = atoi(argv[startListOfWeights + idx]);

		// Temporary rasters
		GENERATE_RASTER_WITH_NAME_TEMPLATE(croplandOrdersVector[idx], "%s%s%02d", runParams.tmpDir.c_str(), "tmp_orders_", idx);
		GENERATE_RASTER_WITH_NAME_TEMPLATE(croplandWeightsVector[idx], "%s%s%02d", runParams.tmpDir.c_str(), "tmp_weights_", idx);
		GENERATE_RASTER_WITH_NAME_TEMPLATE(croplandWeightsVector2[idx], "%s%s%02d", runParams.tmpDir.c_str(), "tmp_weights2_", idx);
		
		getBackVector[idx + numRasters * 0] = croplandOrdersVector[idx];
		getBackVector[idx + numRasters * 1] = croplandWeightsVector[idx];
		getBackVector[idx + numRasters * 2] = croplandWeightsVector2[idx];
	}

	multipleRasterArithmetics(&processListOfRasters, croplandRastersVector, getBackVector, (void *)priorityData);

	// Free up memory
	for (size_t idx = 0; idx < (size_t)numRasters; idx++)
	{
		delete [] croplandRastersVector[idx];
		delete [] croplandOrdersVector[idx];
		delete [] croplandWeightsVector[idx];
		delete [] croplandWeightsVector2[idx];
	}
	delete [] priorityData;

	printf("End: ");
	outputLocalTime();
	timer.stop();
	printf("Elapsed time: %5.2f seconds.\n", timer.elapsedSeconds());

	return 0;
}