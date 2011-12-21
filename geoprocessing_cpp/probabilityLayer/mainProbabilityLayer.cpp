#include <iostream>
#include <vector>
// additional include directory: $(PYTHON_INCLUDE)
//#include <Python.h>

#include "raster.h"
#include "timer.h"

#define GENERATE_RASTER_WITH_NAME_TEMPLATE(RASTER_NAME, NAME_TEMPLATE, ...)		\
	{																			\
		char tmpName[1000];														\
		sprintf(tmpName, NAME_TEMPLATE, __VA_ARGS__);							\
		string tmpNameStr = string(tmpName);									\
		RASTER_NAME = new raster(tmpNameStr, raster::TEMPORARY);				\
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

	//raster areaRaster(argv[4], raster::INPUT);
	//raster countriesRaster(argv[5], raster::INPUT);
	// TODO: STOPGAP:
	raster areaRaster(argv[4], raster::TEMPORARY);
	raster countriesRaster(argv[5], raster::TEMPORARY);
	// END STOPGAP

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
	vector<int> prioritiesVector;
	vector<int> prioritiesVector2;
	vector<int> weightsVector;

	croplandRastersVector.resize(numRasters);
	prioritiesVector.resize(numRasters);
	prioritiesVector2.resize(numRasters);
	weightsVector.resize(numRasters);

	// Temporary raster vectors
	vector<raster *> croplandIsDataVector;

	croplandIsDataVector.resize(numRasters);

	// Initializing vector of cropland rasters and related vectors
	for (size_t idx = 0; idx < (size_t)numRasters; idx++)
	{
		//croplandRastersVector[idx] = new raster(argv[startListOfRasters + idx], raster::INPUT);
		// TODO: STOPGAP
		croplandRastersVector[idx] = new raster(argv[startListOfRasters + idx], raster::TEMPORARY);
		// END STOPGAP
		prioritiesVector[idx] = atoi(argv[startListOfPriorities + idx]);
		prioritiesVector2[idx] = atoi(argv[startListOfPriorities2 + idx]);
		weightsVector[idx] = atoi(argv[startListOfWeights + idx]);

		// Temporary rasters
		GENERATE_RASTER_WITH_NAME_TEMPLATE(croplandIsDataVector[idx], "%s%s%02d", runParams.tmpDir.c_str(), "tmp_ones_", idx);
	}



	// Free up memory
	for (size_t idx = 0; idx < (size_t)numRasters; idx++)
	{
		//delete [] croplandRastersVector[idx];
		//delete [] croplandIsDataVector[idx];
	}

	printf("End: ");
	outputLocalTime();
	timer.stop();
	printf("Elapsed time: %5.2f seconds.\n", timer.elapsedSeconds());

	return 0;
}