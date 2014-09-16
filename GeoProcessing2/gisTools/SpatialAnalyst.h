#pragma once

#include "errorCodes.h"
#include "raster.h"

class SpatialAnalyst
{
public:
	SpatialAnalyst(void);
	~SpatialAnalyst(void);

	static void rasterArithmetics(float (*func)(float, float), 
		const raster & inRaster, 
		const float num, 
		raster & outRaster);

	static void rasterArithmetics(float (*func)(float, float), 
		const raster & inRaster1, 
		const raster & inRaster2, 
		raster & outRaster);
};
