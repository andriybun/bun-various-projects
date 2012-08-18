#include <iostream>
#include <vector>
// additional include directory: $(PYTHON_INCLUDE)
//#include <Python.h>

#include "raster.h"
#include "timer.h"

float preprocessCellAreas(float area, float stat)
{
	return (area / (float)10) * (stat / (float)100);
}

int main(int argc, char * argv[])
{
	ASSERT_INT(argc == 8, INCORRECT_INPUT_PARAMS);

	// Command line arguments:
	// 1 - workingDir
	// 2 - resultDir
	// 3 - tmpDir
	// 4 - areaGrid
	// 5 - zones grid
	// 6 - cropland grid
	// 7 - output

	printf("Start: ");
	outputLocalTime();
	Timer timer;
	timer.start();

	runParamsT runParams;
	runParams.workingDir = string(argv[1]) + "\\";
	runParams.resultDir = string(argv[2]) + "\\";
	runParams.tmpDir = string(argv[3]) + "\\";

	raster areaRaster(argv[4], raster::INPUT);
	raster zonesRaster(argv[5], raster::INPUT);
	raster croplandRaster(argv[6], raster::INPUT);
	raster output(argv[7], raster::OUTPUT);

	// Product of cell area and cropland percentage
	raster tmpCellAreaStat(runParams.tmpDir + "tmp_cell_area_stat", raster::TEMPORARY);
	areaRaster.rasterArithmetics(&preprocessCellAreas, croplandRaster, tmpCellAreaStat);
	tmpCellAreaStat.zonalStatistics(zonesRaster, output, raster::SUM);

	printf("End: ");
	outputLocalTime();
	timer.stop();
	printf("Elapsed time: %5.2f seconds.\n", timer.elapsedSeconds());

	return 0;
}