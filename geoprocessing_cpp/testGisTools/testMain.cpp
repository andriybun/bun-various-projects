#include <iostream>
#include <vector>

#include "raster.h"

float processRasters(float val1, float val2)
{
	return (val1 + 2) * val2;
}

float processMultipleRasters(const vector<float> & vec)
{
	float sum = 0;
	for (size_t idx = 0; idx < vec.size(); idx++)
	{
		sum += vec[idx];
	}
	return sum;
}

float preprocessCellAreas(const vector<float> & vec)
{
	return (vec[0] / (float)10) * (vec[1] / (float)100);
}

float preprocessCellAreas(float area, float stat)
{
	return (area / (float)10) * (stat / (float)100);
}

int main()
{
	printf(__TIME__ "\n");
	bool deleteFloats = true;
	runParamsT runParams;
	runParams.resultDir = "E:\\GIS\\cropland calibrated\\";
	runParams.tmpDir = "E:\\GIS\\cropland calibrated\\tmp\\";

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
	//////////////////////////////////////////////////////////////////////////

	//raster inRaster("E:\\GIS\\img\\glob");
	//raster inZoneRaster("E:\\GIS\\float\\glc");
	//raster sumRaster("E:\\GIS\\float\\zs_sum");
	//raster meanRaster("E:\\GIS\\float\\zs_mean");
	//raster countRaster("E:\\GIS\\float\\zs_count");
	//raster multipleArithmeticsResultRaster("E:\\GIS\\float\\xxx_ma_result");

	//inRaster.zonalStatistics(inZoneRaster, sumRaster, raster::SUM);
	//inRaster.zonalStatistics(inZoneRaster, meanRaster, raster::MEAN);
	//inRaster.zonalStatistics(inZoneRaster, countRaster, raster::COUNT);

	//vector<raster *> rasterVector;
	//rasterVector.push_back(sumRaster);
	//rasterVector.push_back(meanRaster);
	//rasterVector.push_back(countRaster);
	//
	//multipleRasterArithmetics(&processMultipleRasters, rasterVector, multipleArithmeticsResultRaster);
	//sumRaster.convertFloatToRaster();
	//meanRaster.convertFloatToRaster();
	//countRaster.convertFloatToRaster();
	//multipleArithmeticsResultRaster.convertFloatToRaster();

	//return 0;
}