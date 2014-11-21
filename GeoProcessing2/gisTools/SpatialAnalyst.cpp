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
															const std::vector<std::shared_ptr<raster>> & inRastersVector,
															std::vector<std::shared_ptr<raster>> & outRastersVector)
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
	for (auto r : inRastersVector)
	{
		r->validateExtent(inRastersVector[0]->extent);
		inFileVector.push_back(new BigFileIn(*r));
	}

	printf("Executing multiple raster arithmetics\n");

	// Copy header files for all output rasters
	size_t idx = 0;
	for(auto r: outRastersVector)
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
	for(BigFileIn* f: inFileVector) delete f;
	for(BigFileOut* f: outFileVector) delete f;
}

void SpatialAnalyst::ZonalStatisticsAsTable(const raster &zoneRaster
											, const raster &valueRaster
											, SpatialAnalyst::zonalStatisticsTableT &statTable
											//, SpatialAnalyst::statisticsTypeT statType
											)
{
	zoneRaster.validateExtent(valueRaster.extent);

	printf("Executing zonal statistics as table\n");

	rasterBufT zoneBuf, valueBuf;

	BigFileIn zoneFile(zoneRaster);
	BigFileIn valueFile(valueRaster);

	while (zoneFile.read(zoneBuf))
	{
		valueFile.read(valueBuf);
		for (int i = 0; i < zoneBuf.nEl; i++)
		{
			if ((zoneBuf.buf[i] != zoneBuf.noDataValue) && (valueBuf.buf[i] != valueBuf.noDataValue))
			{
				auto it = statTable.find((int)zoneBuf.buf[i]);
				if (it == statTable.end())
				{
					auto insPair = statTable.insert(
						zonalStatisticsRecordT((int)zoneBuf.buf[i], statisticsStructT()));
					it = insPair.first;
				}
				auto zoneData = &it->second;
				zoneData->count++;
				zoneData->countNonZero += (int)(valueBuf.buf[i] == 0);
				zoneData->maxVal = xmax(zoneData->maxVal, valueBuf.buf[i]);
				zoneData->minVal = xmin(zoneData->minVal, valueBuf.buf[i]);
				zoneData->sumVal += valueBuf.buf[i];
			}
		}
		zoneFile.printProgress();
	}

	for (auto it = statTable.begin(); it != statTable.end(); ++it)
	{
		it->second.meanVal = it->second.sumVal / it->second.count;
	}
}

void SpatialAnalyst::ZonalStatistics(const raster &zoneRaster,
									 const raster &valueRaster,
									 raster &outRaster,
									 statisticsTypeT statisticType
									 )
{
	zoneRaster.validateExtent(valueRaster.extent);

	printf("Executing zonal statistics\n");

	zonalStatisticsTableT statTable;
	ZonalStatisticsAsTable(zoneRaster, valueRaster, statTable);

	raster::copyFile(zoneRaster.getHdrPath(), outRaster.getHdrPath());
	outRaster.copyProperties(zoneRaster);

	rasterBufT zoneBuf, valueBuf, outBuf;

	BigFileIn zoneFile(zoneRaster);
	BigFileIn valueFile(valueRaster);
	BigFileOut outFile(outRaster);

	while (zoneFile.read(zoneBuf))
	{
		valueFile.read(valueBuf);

		rasterBufT outBuf;
		outBuf.buf.allocate(zoneBuf.nEl);
		outBuf.nEl = zoneBuf.nEl;
		outBuf.noDataValue = zoneBuf.noDataValue;

		for (int i = 0; i < zoneBuf.nEl; i++)
		{
			if ((zoneBuf.buf[i] != zoneBuf.noDataValue) && (valueBuf.buf[i] != valueBuf.noDataValue))
			{
				switch (statisticType)
				{
				case SUM:
					outBuf.buf[i] = statTable[(int)zoneBuf.buf[i]].sumVal;
					break;
				case MEAN:
					outBuf.buf[i] = statTable[(int)zoneBuf.buf[i]].meanVal;
					break;
				case MIN:
					outBuf.buf[i] = statTable[(int)zoneBuf.buf[i]].minVal;
					break;
				case MAX:
					outBuf.buf[i] = statTable[(int)zoneBuf.buf[i]].maxVal;
					break;
				case COUNT:
					outBuf.buf[i] = (float)statTable[(int)zoneBuf.buf[i]].count;
					break;
				case COUNT_NON_ZERO:
					outBuf.buf[i] = (float)statTable[(int)zoneBuf.buf[i]].countNonZero;
				}
			}
			else
			{
				outBuf.buf[i] = outBuf.noDataValue;
			}
		}
		outFile.write(outBuf);
		zoneFile.printProgress();
	}
}