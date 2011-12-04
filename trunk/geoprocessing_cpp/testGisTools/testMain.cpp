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

int main()
{
	raster areaRaster = raster("E:\\GIS\\cropland data\\area_grid");
	raster classRaster = raster("E:\\GIS\\cropland data\\prob_classes");
	raster inZoneRaster = raster("E:\\GIS\\cropland data\\countries");

	raster::tableT probabilitySummary;
	
	areaRaster.zonalSumByClassAsTable(inZoneRaster, classRaster, probabilitySummary);

	return 0;
	//////////////////////////////////////////////////////////////////////////

	//raster inRaster = raster("E:\\GIS\\img\\glob");
	//raster inZoneRaster = raster("E:\\GIS\\float\\glc");
	//raster sumRaster = raster("E:\\GIS\\float\\zs_sum");
	//raster meanRaster = raster("E:\\GIS\\float\\zs_mean");
	//raster countRaster = raster("E:\\GIS\\float\\zs_count");
	//raster multipleArithmeticsResultRaster = raster("E:\\GIS\\float\\xxx_ma_result");

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