#pragma once

#include <string>
#include <vector>
#include <boost/foreach.hpp>

#include "errorCodes.h"
#include "raster.h"

#define _foreach	BOOST_FOREACH

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

	static void multipleRasterArithmetics(
		void (*func)(
			const std::vector<float> &, 
			const std::vector<float> &, 
			const std::vector<float> &, 
			std::vector<float> &
		),
		const std::vector<raster> & inRastersVector,
		std::vector<raster> & outRastersVector);

};
