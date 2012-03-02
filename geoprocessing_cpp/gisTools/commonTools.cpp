#include <string>

#include "commonTools.h"

float threshold(float val, float constThreshold)
{
	return (val > constThreshold) ? (float)1 : (float)0;
}

void getAreasWithCropland(raster & inZoneRaster,
						  raster & inCroplandRaster,
						  raster & isDataRaster,
						  float selectionThreshold,
						  const runParamsT & runParams)
{
	std::string tmpCountPath = runParams.tmpDir + "tmp_count";
	std::string tmpCountNonzeroPath = runParams.tmpDir + "tmp_count_nonzero";
	std::string tmpRatioPath = runParams.tmpDir + "tmp_ratio";
	raster tmpCount(tmpCountPath.c_str(), raster::TEMPORARY);
	raster tmpCountNonzero(tmpCountNonzeroPath.c_str(), raster::TEMPORARY);
	raster tmpRatio(tmpRatioPath.c_str(), raster::TEMPORARY);
	inCroplandRaster.zonalStatistics(inZoneRaster, tmpCount, raster::COUNT);
	inCroplandRaster.zonalStatistics(inZoneRaster, tmpCountNonzero, raster::COUNT_NON_ZERO);
	
	tmpCountNonzero.rasterArithmetics(&xdivide, tmpCount, tmpRatio);
	tmpRatio.rasterArithmetics(&threshold, selectionThreshold, isDataRaster);
}

