#pragma once

#include <map>
#include <string>
#include <vector>
#include <cfloat>

#include "errorCodes.h"
#include "raster.h"

class SpatialAnalyst
{
public:
	SpatialAnalyst(void);
	~SpatialAnalyst(void);

	// Types definition
	enum statisticsTypeT
	{
		SUM,
		MEAN,
		MIN,
		MAX,
		COUNT,
		COUNT_NON_ZERO
	};
	struct statisticsStructT
	{
		float sumVal;
		float minVal;
		float maxVal;
		float meanVal;
		int count;
		int countNonZero;
		statisticsStructT()
			: count(0)
			, countNonZero(0)
			, sumVal(0)
			, minVal(FLT_MAX)
			, maxVal(FLT_MIN) 
		{};
	};
	typedef std::pair<int, statisticsStructT> zonalStatisticsRecordT;
	typedef std::map<int, statisticsStructT> zonalStatisticsTableT;

	// Spatial analyst methods
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
		const raster &zoneRaster
		, const raster &valueRaster
		, zonalStatisticsTableT &statTable
		//, statisticsTypeT statType
		);

	static void ZonalStatistics(
		const raster &zoneRaster,
		const raster &valueRaster,
		raster &outRaster,
		statisticsTypeT statisticType
		);

};
