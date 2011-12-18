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

float postprocessResults(float calibratedResult, float stat)
{
	return stat;
}

int main(int argc, char * argv[])
{
	ASSERT_INT(argc == 11, INCORRECT_INPUT_PARAMS);

	// Command line arguments:
	// 1 - resultDir
	// 2 - tmpDir
	// 3 - areaGrid
	// 4 - statisticsLevel0
	// 5 - statisticsLevel1
	// 6 - statisticsLevel2
	// 7 - probabilityGrid
	// 8 - statGrid
	// 9 - output

	printf("Start: ");
	outputLocalTime();
	Timer timer;
	timer.start();

	bool deleteFloats = true;

	runParamsT runParams;
	runParams.workingDir = string(argv[1]) + "\\";
	runParams.resultDir = string(argv[2]) + "\\";
	runParams.tmpDir = string(argv[3]) + "\\";

	// TODO: specify if input/output and validate existance of file
	// TODO: put float grids to temporary folder
	raster areaRaster(argv[4]);
	raster statisticsRasterLevel0(argv[5]);
	raster statisticsRasterLevel1(argv[6]);
	raster statisticsRasterLevel2(argv[7]);
	raster probabilityRaster(argv[8]);
	raster statRaster(argv[9]);
	raster output(argv[10]);

	// Results for levels:
	raster outCroplandRasterLevel0(runParams.resultDir + "validated_cropland_level0", deleteFloats);
	raster outCroplandRasterLevel1(runParams.resultDir + "validated_cropland_level1", deleteFloats);
	raster outCroplandRasterLevel2(runParams.resultDir + "validated_cropland_level2", deleteFloats);

	// Difference between results and reported statistics:
	raster outErrorRasterLevel0(runParams.tmpDir + "validated_cropland_error_level0", deleteFloats);
	raster outErrorRasterLevel1(runParams.tmpDir + "validated_cropland_error_level1", deleteFloats);
	raster outErrorRasterLevel2(runParams.tmpDir + "validated_cropland_error_level2", deleteFloats);

	// Calibrated results:
	raster outCalibratedRasterLevel1(runParams.resultDir + "calibrated_cropland_level1", deleteFloats);
	raster outCalibratedRasterLevel2(runParams.resultDir + "calibrated_cropland_level2", deleteFloats);

	// Temporary:
	// Product of cell area and cropland percentage
	raster tmpCellAreaStat(runParams.tmpDir + "tmp_cell_area_stat", deleteFloats);

	areaRaster.rasterArithmetics(&preprocessCellAreas, statRaster, tmpCellAreaStat);
	validateCropland(
		tmpCellAreaStat,
		statisticsRasterLevel0,
		probabilityRaster,
		outCroplandRasterLevel0,
		outErrorRasterLevel0);
	validateCropland(
		tmpCellAreaStat,
		statisticsRasterLevel1,
		probabilityRaster,
		outCroplandRasterLevel1,
		outErrorRasterLevel1);
	calibrateCropland(
		tmpCellAreaStat,
		probabilityRaster,
		statisticsRasterLevel0,
		statisticsRasterLevel1,
		outCroplandRasterLevel0,
		outCroplandRasterLevel1,
		outCalibratedRasterLevel1,
		runParams);
	validateCropland(
		tmpCellAreaStat,
		statisticsRasterLevel2,
		probabilityRaster,
		outCroplandRasterLevel2,
		outErrorRasterLevel2);
	calibrateCropland(
		tmpCellAreaStat,
		probabilityRaster,
		statisticsRasterLevel1,		// TODO: validate with level 1 then change to statisticsRasterLevel0,
		statisticsRasterLevel2,
		outCroplandRasterLevel1,
		outCroplandRasterLevel2,
		outCalibratedRasterLevel2,
		runParams);
	outCalibratedRasterLevel2.rasterArithmetics(&postprocessResults, statRaster, output);

	outCroplandRasterLevel0.convertFloatToRaster();
	outCroplandRasterLevel1.convertFloatToRaster();
	outCroplandRasterLevel2.convertFloatToRaster();
	outCalibratedRasterLevel1.convertFloatToRaster();
	outCalibratedRasterLevel2.convertFloatToRaster();
	output.convertFloatToRaster();

	printf("End: ");
	outputLocalTime();
	timer.stop();
	printf("Elapsed time: %5.2f seconds.\n", timer.elapsedSeconds());

	return 0;
}