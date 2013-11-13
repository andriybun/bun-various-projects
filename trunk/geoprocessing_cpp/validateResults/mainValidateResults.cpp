#include <iostream>
#include <vector>

#include "raster.h"
#include "timer.h"

int main(int argc, char * argv[])
{
	ASSERT_INT(argc == 7, INCORRECT_INPUT_PARAMS);

	// Command line arguments:
	// 1 - workingDir
	// 2 - resultDir
	// 3 - tmpDir
	// 4 - rasterToValidate
	// 5 - areaGrid
	// 6 - nationalStatistics

	printf("Start: ");
	outputLocalTime();
	Timer timer;
	timer.start();

	runParamsT runParams;
	runParams.workingDir = string(argv[1]) + "\\";
	runParams.resultDir = string(argv[2]) + "\\";
	runParams.tmpDir = string(argv[3]) + "\\";

	raster rasterToValidate(argv[4], raster::INPUT);
	raster areaRaster(argv[5], raster::INPUT);
	raster nationalStatistics(argv[6], raster::INPUT);

	raster outTotalCroplandRaster(runParams.resultDir + "total_out_cropland", raster::OUTPUT);
	raster outAbsDiffRaster(runParams.resultDir + "error_abs", raster::OUTPUT);
	raster outRelDiffRaster(runParams.resultDir + "error_rel", raster::OUTPUT);

	validateResult(
		areaRaster,
		rasterToValidate,
		nationalStatistics,
		outTotalCroplandRaster,
		outAbsDiffRaster,
		outRelDiffRaster,
		runParams);

	printf("End: ");
	outputLocalTime();
	timer.stop();
	printf("Elapsed time: %5.2f seconds.\n", timer.elapsedSeconds());

	return 0;
}