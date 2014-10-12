#pragma once

#include <string>
#include <vector>
#include <boost/foreach.hpp>

#include "errorCodes.h"
#include "raster.h"

#define foreach_	BOOST_FOREACH

class SpatialAnalyst
{
public:
	SpatialAnalyst(void);
	~SpatialAnalyst(void);

	enum statisticsTypeT
	{
		SUM,
		MEAN,
		MIN,
		MAX,
		COUNT,
		COUNT_NON_ZERO
	};

	static void RasterArithmetics(float (*func)(float, float), 
		const raster & inRaster, 
		const float num, 
		raster & outRaster);

	static void RasterArithmetics(float (*func)(float, float), 
		const raster & inRaster1, 
		const raster & inRaster2, 
		raster & outRaster);

	static void MultipleRasterArithmetics(
		void (*func)(
			const std::vector<float> &, 
			const std::vector<float> &, 
			const std::vector<float> &, 
			std::vector<float> &
		),
		const std::vector<raster*> & inRastersVector,
		std::vector<raster*> & outRastersVector);

	static void ZonalStatisticsAsTable(
		const raster &zoneRaster,
		const raster &valueRaster,
		//<some_table_type> &outTable,
		//statisticsTypeT statisticType,
		bool ignoreNodata
		);

	static void ZonalStatistics(
		const raster &zoneRaster,
		const raster &valueRaster,
		raster &outRaster,
		statisticsTypeT statisticType,
		bool ignoreNodata
		);

};
