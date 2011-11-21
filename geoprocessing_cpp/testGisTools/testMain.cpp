#include <iostream>

#include "raster.h"

float processRasters(float val1, float val2)
{
	return (val1 + 2) * val2;
}

int main()
{
	//raster inRaster = raster("E:\\GIS\\float\\zero");
	//raster inRaster2 = raster("E:\\GIS\\float\\three");
	//raster sumRaster = raster("E:\\GIS\\float\\sum");
	//inRaster.copy(sumRaster);
	//inRaster.rasterArithmetics(&processRasters, inRaster2, sumRaster);

	raster inRaster = raster("E:\\GIS\\float\\glob");
	raster inZoneRaster = raster("E:\\GIS\\float\\glc");
	raster sumRaster = raster("E:\\GIS\\float\\zs_sum");
	raster meanRaster = raster("E:\\GIS\\float\\zs_mean");
	raster countRaster = raster("E:\\GIS\\float\\zs_count");

	inRaster.zonalStatistics(inZoneRaster, sumRaster, raster::SUM);
	inRaster.zonalStatistics(inZoneRaster, meanRaster, raster::MEAN);
	inRaster.zonalStatistics(inZoneRaster, countRaster, raster::COUNT);

	return 0;
}