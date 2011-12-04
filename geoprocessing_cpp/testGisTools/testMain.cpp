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

int main()
{
	bool deleteFloats = true;
	raster areaRaster("E:\\GIS\\cropland data\\area_grid");
	raster statRaster("E:\\GIS\\cropland data\\cl_mean");
	raster classRaster("E:\\GIS\\cropland data\\prob_classes");
	raster inZoneRaster("E:\\GIS\\cropland data\\countries");
	raster outCroplandRaster("E:\\GIS\\cropland data\\validated_cropland", deleteFloats);
	raster outErrorRaster("E:\\GIS\\cropland data\\validated_cropland_error", deleteFloats);

	raster tmpCellAreaStat("E:\\GIS\\cropland data\\tmp_cell_area_min", deleteFloats);

	vector<raster> rasterVector;
	rasterVector.push_back(areaRaster);
	rasterVector.push_back(statRaster);
	
	multipleRasterArithmetics(&preprocessCellAreas, rasterVector, tmpCellAreaStat);
	tmpCellAreaStat.validateCropland(inZoneRaster, classRaster, outCroplandRaster, outErrorRaster);

	outCroplandRaster.convertFloatToRaster();
	outErrorRaster.convertFloatToRaster();

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

	//vector<raster> rasterVector;
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