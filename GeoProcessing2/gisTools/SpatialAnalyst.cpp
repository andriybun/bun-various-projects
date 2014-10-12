#include "SpatialAnalyst.h"

SpatialAnalyst::SpatialAnalyst(void)
{
}

SpatialAnalyst::~SpatialAnalyst(void)
{
}

//////////////////////////////////////////////////////////////////////////
// Spatial analyst tools
//////////////////////////////////////////////////////////////////////////

void SpatialAnalyst::RasterArithmetics(float (*func)(float, float), 
									   const raster & inRaster,
									   const float num,
									   raster & outRaster)
{
	printf("Executing raster arithmetics\n");

	raster::copyFile(inRaster.getHdrPath(), outRaster.getHdrPath());
	outRaster.copyProperties(inRaster);
	rasterBufT inBuf;

	BigFileIn inFile(inRaster);
	BigFileOut outFile(outRaster);

	while (inFile.read(inBuf))
	{
		rasterBufT outBuf;
		outBuf.buf.allocate(inBuf.nEl);
		outBuf.nEl = inBuf.nEl;
		outBuf.noDataValue = inBuf.noDataValue;
		for (int i = 0; i < inBuf.nEl; i++)
		{
			if (inBuf.buf[i] != inBuf.noDataValue)
			{
				outBuf.buf[i] = func(inBuf.buf[i], num);
			}
			else
			{
				outBuf.buf[i] = outBuf.noDataValue;
			}
		}
		outFile.write(outBuf);
		inFile.printProgress();
	}
}

void SpatialAnalyst::RasterArithmetics(float (*func)(float, float), 
									   const raster & inRaster1,
									   const raster & inRaster2,
									   raster & outRaster)
{
	inRaster1.validateExtent(inRaster2.extent);

	printf("Executing raster arithmetics\n");

	raster::copyFile(inRaster1.getHdrPath(), outRaster.getHdrPath());
	outRaster.copyProperties(inRaster1);
	rasterBufT inBuf1, inBuf2, outBuf;

	BigFileIn inFile1(inRaster1);
	BigFileIn inFile2(inRaster2);
	BigFileOut outFile(outRaster);

	while (inFile1.read(inBuf1))
	{
		inFile2.read(inBuf2);
		rasterBufT outBuf;
		outBuf.buf.allocate(inBuf1.nEl);
		outBuf.nEl = inBuf1.nEl;
		outBuf.noDataValue = inBuf1.noDataValue;
		for (int i = 0; i < inBuf1.nEl; i++)
		{
			if ((inBuf1.buf[i] != inBuf1.noDataValue) && (inBuf2.buf[i] != inBuf2.noDataValue))
			{
				outBuf.buf[i] = func(inBuf1.buf[i], inBuf2.buf[i]);
			}
			else
			{
				outBuf.buf[i] = outBuf.noDataValue;
			}
		}
		outFile.write(outBuf);
		inFile1.printProgress();
	}
}

void SpatialAnalyst::MultipleRasterArithmetics(void (*func)(const std::vector<float> &, 
															const std::vector<float> &, 
															const std::vector<float> &, 
															std::vector<float> & ),
											   const std::vector<raster*> & inRastersVector,
											   std::vector<raster*> & outRastersVector)
{
	size_t numInRasters = inRastersVector.size();
	size_t numOutRasters = outRastersVector.size();

	std::vector<BigFileIn*> inFileVector;
	std::vector<BigFileOut*> outFileVector;

	// Read buffers
	std::vector<rasterBufT> inBufVector, outBufVector;
	inBufVector.resize(numInRasters);
	outBufVector.resize(numOutRasters);

	// Validate extent for all input rasters
	foreach_(raster* r, inRastersVector)
	{
		r->validateExtent(inRastersVector[0]->extent);
		inFileVector.push_back(new BigFileIn(*r));
	}

	printf("Executing multiple raster arithmetics\n");

	// Copy header files for all output rasters
	size_t idx = 0;
	foreach_(raster* r, outRastersVector)
	{
		r->copyProperties(*inRastersVector[0]);
		raster::copyFile(inRastersVector[0]->getHdrPath(), r->getHdrPath());
		outFileVector.push_back(new BigFileOut(*r));
		outBufVector[idx++].noDataValue = r->noDataValue;
	}

	// Main loop
	while (inFileVector[0]->read(inBufVector[0]))
	{
		for (size_t rasterIdx = 1; rasterIdx < numInRasters; rasterIdx++)
		{
			inFileVector[rasterIdx]->read(inBufVector[rasterIdx]);
		}
		std::vector<float> noDataValuesVector, noDataOutValuesVector;
		for (size_t rasterIdx = 0; rasterIdx < numInRasters; rasterIdx++)
		{
			noDataValuesVector.push_back(inBufVector[rasterIdx].noDataValue);
		}
		for (size_t rasterIdx = 0; rasterIdx < numOutRasters; rasterIdx++)
		{
			noDataOutValuesVector.push_back(outBufVector[rasterIdx].noDataValue);
			outBufVector[rasterIdx].nEl = inBufVector[0].nEl;
			outBufVector[rasterIdx].buf.allocateOnce(outBufVector[rasterIdx].nEl);
		}
		std::vector<float> passArg, result;
		passArg.resize(numInRasters);
		result.resize(numOutRasters);

		// Processing loop
		for (int i = 0; i < inBufVector[0].nEl; i++)
		{
			for (size_t rasterIdx = 0; rasterIdx < numInRasters; rasterIdx++)
			{
				passArg[rasterIdx] = inBufVector[rasterIdx].buf[i];
			}

			// Call function
			func(passArg, noDataValuesVector, noDataOutValuesVector, result);

			// Results to out buffer
			for (size_t idx = 0; idx < numOutRasters; idx++)
			{
				outBufVector[idx].buf[i] = result[idx];
			}	
		}
		// Write all out buffers to files
		for (size_t idx = 0; idx < numOutRasters; idx++)
		{
			outFileVector[idx]->write(outBufVector[idx]);
		}
		inFileVector[0]->printProgress();
	}

	// Cleanup
	foreach_(BigFileIn* f, inFileVector) delete f;
	foreach_(BigFileOut* f, outFileVector) delete f;
}

void SpatialAnalyst::ZonalStatisticsAsTable(const raster &zoneRaster,
											const raster &valueRaster,
											// <some_table_type> &outTable,
											//statisticsTypeT statisticType,
											bool ignoreNodata
											)
{
	// Implement
}

void SpatialAnalyst::ZonalStatistics(const raster &zoneRaster,
									 const raster &valueRaster,
									 raster &outRaster,
									 statisticsTypeT statisticType,
									 bool ignoreNodata
									 )
{
	// Implement
}