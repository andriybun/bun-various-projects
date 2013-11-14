#include <iostream>
#include <vector>
// additional include directory: $(PYTHON_INCLUDE)
//#include <Python.h>

#include "raster.h"
#include "timer.h"

float preprocessCellAreas(float area, float stat)
{
	return area * stat / (float)1000;
}

float postprocessResults(float calibratedResult, float stat)
{
	return stat;
}

int main(int argc, char * argv[])
{
	ASSERT_INT(argc == 11, INCORRECT_INPUT_PARAMS);

	// Command line arguments:
	// 1 - workingDir
	// 2 - resultDir
	// 3 - tmpDir
	// 4 - areaGrid
	// 5 - statisticsLevel0
	// 6 - statisticsLevel1
	// 7 - statisticsLevel2
	// 8 - probabilityGrid
	// 9 - statGrid
	// 10 - output

	printf("Start: ");
	outputLocalTime();
	Timer timer;
	timer.start();

	runParamsT runParams;
	runParams.workingDir = string(argv[1]) + "\\";
	runParams.resultDir = string(argv[2]) + "\\";
	runParams.tmpDir = string(argv[3]) + "\\";

	raster areaRaster(argv[4], raster::INPUT);
	raster statisticsRasterLevel0(argv[5], raster::INPUT);
	raster statisticsRasterLevel1(argv[6], raster::INPUT);
	raster statisticsRasterLevel2(argv[7], raster::INPUT);
	raster probabilityRaster(argv[8], raster::INPUT);
	raster statRaster(argv[9], raster::INPUT);
	//raster output(argv[10], raster::OUTPUT);

	// Results for levels:
	raster outCroplandRasterLevel0(runParams.resultDir + "validated_cropland_level0", raster::OUTPUT);
	//raster outCroplandRasterLevel1(runParams.resultDir + "validated_cropland_level1", raster::OUTPUT);
	//raster outCroplandRasterLevel2(runParams.resultDir + "validated_cropland_level2", raster::OUTPUT);

	//// Difference between results and reported statistics:
	//raster outErrorRasterLevel0(runParams.tmpDir + "validated_cropland_error_level0", raster::TEMPORARY);
	//raster outErrorRasterLevel1(runParams.tmpDir + "validated_cropland_error_level1", raster::TEMPORARY);
	//raster outErrorRasterLevel2(runParams.tmpDir + "validated_cropland_error_level2", raster::TEMPORARY);

	//// Calibrated results:
	//raster outCalibratedRasterLevel1(runParams.resultDir + "calibrated_cropland_level1", raster::OUTPUT);
	//raster outCalibratedRasterLevel2(runParams.resultDir + "calibrated_cropland_level2", raster::OUTPUT);

	// Error analysis:
	raster outTotalCroplandRaster(runParams.resultDir + "total_out_cropland", raster::OUTPUT);
	raster outAbsDiffRaster(runParams.resultDir + "error_abs", raster::OUTPUT);
	raster outRelDiffRaster(runParams.resultDir + "error_rel", raster::OUTPUT);
	//raster outMinClassRaster(runParams.resultDir + "min_class", raster::TEMPORARY);

	// Temporary:
	// Product of cell area and cropland percentage
	raster tmpCellAreaStat(runParams.tmpDir + "tmp_cell_area_stat", raster::TEMPORARY);

	areaRaster.rasterArithmetics(&preprocessCellAreas, statRaster, tmpCellAreaStat);
	validateCropland(
		statRaster,
		tmpCellAreaStat,
		statisticsRasterLevel0,
		probabilityRaster,
		outCroplandRasterLevel0);
	validateResult(
		areaRaster,
		outCroplandRasterLevel0,
		statisticsRasterLevel0,
		outTotalCroplandRaster,
		outAbsDiffRaster,
		outRelDiffRaster,
		runParams);
	
	//validateCropland(
	//	statRaster,
	//	tmpCellAreaStat,
	//	statisticsRasterLevel1,
	//	probabilityRaster,
	//	outCroplandRasterLevel1);
	//calibrateCropland(
	//	statRaster,
	//	tmpCellAreaStat,
	//	probabilityRaster,
	//	statisticsRasterLevel0,
	//	statisticsRasterLevel1,
	//	outCroplandRasterLevel0,
	//	outCroplandRasterLevel1,
	//	outCalibratedRasterLevel1,
	//	runParams);
	//validateCropland(
	//	statRaster,
	//	tmpCellAreaStat,
	//	statisticsRasterLevel2,
	//	probabilityRaster,
	//	outCroplandRasterLevel2);
	//calibrateCropland(
	//	statRaster,
	//	tmpCellAreaStat,
	//	probabilityRaster,
	//	statisticsRasterLevel0,
	//	statisticsRasterLevel2,
	//	outCalibratedRasterLevel1,
	//	outCroplandRasterLevel2,
	//	outCalibratedRasterLevel2,
	//	outMinClassRaster,
	//	runParams);
	//validateResult(
	//	areaRaster,
	//	outCalibratedRasterLevel2,
	//	statisticsRasterLevel0,
	//	outTotalCroplandRaster,
	//	outAbsDiffRaster,
	//	outRelDiffRaster,
	//	runParams);

	//outCalibratedRasterLevel2.copy(output);

	printf("End: ");
	outputLocalTime();
	timer.stop();
	printf("Elapsed time: %5.2f seconds.\n", timer.elapsedSeconds());

	return 0;
}