#include <iostream>
#include <vector>
// additional include directory: $(PYTHON_INCLUDE)
//#include <Python.h>

#include "raster.h"
#include "timer.h"

const int MAX_LEVELS = 3;

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
	runParams.debugDir = runParams.resultDir + "debug\\";

	raster areaRaster(argv[4], raster::INPUT);
	raster probabilityRaster(argv[8], raster::INPUT);
	raster statRaster(argv[9], raster::INPUT);
	raster output(argv[10], raster::OUTPUT);

	// Product of cell area and cropland percentage
	raster tmpCellAreaStat(runParams.tmpDir + "tmp_cell_area_stat", raster::TEMPORARY);
	areaRaster.rasterArithmetics(&preprocessCellAreas, statRaster, tmpCellAreaStat);

	string levelIdxChar("012");

	// Create vectors of inputs and results
	raster statisticsRasterLevelVector[MAX_LEVELS];
	raster outCroplandRasterLevelVector[MAX_LEVELS];
	raster outCalibratedRasterLevelVector[MAX_LEVELS];
	
	// Debugging
	raster debugMinClassVector[MAX_LEVELS];

	// Process levels
	int levelIdx = 0;
	for (; levelIdx < MAX_LEVELS; levelIdx++)
	{
		statisticsRasterLevelVector[levelIdx].rasterInit(argv[5+levelIdx], raster::INPUT);

		if (statisticsRasterLevelVector[levelIdx].isEmpty())
		{
			ASSERT_INT(levelIdx != 0, INCORRECT_INPUT_PARAMS);
			printf("Processing levels: loop stopped at level %d\n", levelIdx);
			break;
		}

		outCroplandRasterLevelVector[levelIdx].rasterInit(runParams.debugDir + "validated_cropland_level" + levelIdxChar[levelIdx], raster::OUTPUT);
		outCalibratedRasterLevelVector[levelIdx].rasterInit(runParams.debugDir + "calibrated_cropland_level" + levelIdxChar[levelIdx], raster::OUTPUT);
		debugMinClassVector[levelIdx].rasterInit(runParams.debugDir + "min_class_level" + levelIdxChar[levelIdx], raster::DEBUG);

		// Validate cropland
		validateCropland(
			statRaster,
			tmpCellAreaStat,
			statisticsRasterLevelVector[levelIdx],
			probabilityRaster,
			outCroplandRasterLevelVector[levelIdx],
			debugMinClassVector[levelIdx],
			runParams);

		if (levelIdx == 0)
		{
			outCalibratedRasterLevelVector[levelIdx] = outCroplandRasterLevelVector[levelIdx];
		}
		else
		{
			// Run calibration method
			calibrateCropland(
				statRaster,
				tmpCellAreaStat,
				probabilityRaster,
				statisticsRasterLevelVector[0],
				statisticsRasterLevelVector[levelIdx],
				outCalibratedRasterLevelVector[levelIdx-1],
				outCroplandRasterLevelVector[levelIdx],
				outCalibratedRasterLevelVector[levelIdx],
				debugMinClassVector[levelIdx],
				runParams);
		}
	}

	// Error analysis:
	raster outAdjustedResultRaster(runParams.resultDir + "adjusted_output", raster::OUTPUT);
	raster outTotalCroplandRaster(runParams.resultDir + "total_out_cropland", raster::OUTPUT);
	raster outAbsDiffRaster(runParams.resultDir + "error_abs", raster::OUTPUT);
	raster outRelDiffRaster(runParams.resultDir + "error_rel", raster::OUTPUT);

	// Copy latest result to output raster
	if (levelIdx == 1)
	{
		outCroplandRasterLevelVector[0].copy(output);
	}
	else if (levelIdx == 2 || levelIdx == 3)
	{
		outCalibratedRasterLevelVector[levelIdx-1].copy(output);
	}

	// Validation of result
	validateResult(
		areaRaster,
		output,
		statisticsRasterLevelVector[0],
		outTotalCroplandRaster,
		outAbsDiffRaster,
		outRelDiffRaster,
		outAdjustedResultRaster,
		runParams);

	printf("End: ");
	outputLocalTime();
	timer.stop();
	printf("Elapsed time: %5.2f seconds.\n", timer.elapsedSeconds());

	return 0;
}