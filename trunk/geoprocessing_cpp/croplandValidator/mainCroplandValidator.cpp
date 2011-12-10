#include <iostream>
#include <vector>

#include "raster.h"

float preprocessCellAreas(float area, float stat)
{
	return (area / (float)10) * (stat / (float)100);
}

int main(int argc, char * argv[])
{
	ASSERT_INT(argc == 10);

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

	printf(__TIME__ "\n");
	bool deleteFloats = true;
	runParamsT runParams;
	runParams.resultDir = argv[1];
	runParams.tmpDir = argv[2];
	//runParams.resultDir = "E:\\GIS\\cropland calibrated\\";
	//runParams.tmpDir = "E:\\GIS\\cropland calibrated\\tmp\\";

	raster areaRaster("E:\\GIS\\cropland data\\area_grid");
	raster statRaster("E:\\GIS\\cropland data\\cl_mean");
	raster classRaster("E:\\GIS\\cropland data\\prob_classes");
	raster inZoneRasterLevel0("E:\\GIS\\cropland data\\level0_countries");
	raster inZoneRasterLevel1("E:\\GIS\\cropland data\\level1_regions");

	// Results for levels:
	raster outCroplandRasterLevel0(runParams.resultDir + "validated_cropland_level0", deleteFloats);
	raster outCroplandRasterLevel1(runParams.resultDir + "validated_cropland_level1", deleteFloats);

	// Difference between results and reported statistics:
	raster outErrorRasterLevel0(runParams.tmpDir + "validated_cropland_error_level0", deleteFloats);
	raster outErrorRasterLevel1(runParams.tmpDir + "validated_cropland_error_level1", deleteFloats);

	// Calibrated results:
	raster outCalibratedRasterLevel1(runParams.resultDir + "calibrated_cropland_level1", deleteFloats);

	// Temporary:
	// Product of cell area and cropland percentage
	raster tmpCellAreaStat(runParams.tmpDir + "tmp_cell_area_stat", deleteFloats);

	areaRaster.rasterArithmetics(&preprocessCellAreas, statRaster, tmpCellAreaStat);

	validateCropland(
		tmpCellAreaStat,
		inZoneRasterLevel0,
		classRaster,
		outCroplandRasterLevel0,
		outErrorRasterLevel0);
	validateCropland(
		tmpCellAreaStat,
		inZoneRasterLevel1,
		classRaster,
		outCroplandRasterLevel1,
		outErrorRasterLevel1);
	calibrateCropland(
		tmpCellAreaStat,
		classRaster,
		inZoneRasterLevel0,
		inZoneRasterLevel1,
		outCroplandRasterLevel0,
		outCroplandRasterLevel1,
		outCalibratedRasterLevel1,
		runParams);

	outCroplandRasterLevel0.convertFloatToRaster();
	outCroplandRasterLevel1.convertFloatToRaster();
	outCalibratedRasterLevel1.convertFloatToRaster();
	//outErrorRasterLevel0.convertFloatToRaster();
	//outErrorRasterLevel1.convertFloatToRaster();

	printf(__TIME__ "\n");
	return 0;
}