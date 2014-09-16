#include "SpatialAnalyst.h"

SpatialAnalyst::SpatialAnalyst(void)
{
}

SpatialAnalyst::~SpatialAnalyst(void)
{
}

//////////////////////////////////////////////////////////////////////////
// Static methods
//////////////////////////////////////////////////////////////////////////

void SpatialAnalyst::rasterArithmetics(float (*func)(float, float), 
									   const raster & inRaster,
									   const float num,
									   raster & outRaster)
{
	printf("Executing raster arithmetics\n");
	std::string inHdrPath = inRaster.getHdrPath();
	std::string inFltPath = inRaster.getFltPath();
	std::string outHdrPath = outRaster.getHdrPath();
	std::string outFltPath = outRaster.getFltPath();

	raster::copyFile(inHdrPath, outHdrPath);
	inRaster.copyProperties(outRaster);
	rasterBufT inBuf, outBuf;

	BigFileIn inFile(inRaster);
	BigFileOut outFile(outRaster);

	while (inFile.read(inBuf))
	{
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

void SpatialAnalyst::rasterArithmetics(float (*func)(float, float), 
									   const raster & inRaster1,
									   const raster & inRaster2,
									   raster & outRaster)
{
	inRaster1.validateExtent(inRaster2.extent);

	printf("Executing raster arithmetics\n");
	std::string inHdrPath1 = inRaster1.getHdrPath();
	std::string inFltPath1 = inRaster1.getFltPath();
	std::string inHdrPath2 = inRaster2.getHdrPath();
	std::string inFltPath2 = inRaster2.getFltPath();
	std::string outHdrPath = outRaster.getHdrPath();
	std::string outFltPath = outRaster.getFltPath();

	raster::copyFile(inHdrPath1, outHdrPath);
	inRaster1.copyProperties(outRaster);
	rasterBufT inBuf1, inBuf2, outBuf;

	BigFileIn inFile1(inRaster1);
	BigFileIn inFile2(inRaster2);
	BigFileOut outFile(outRaster);

	while (inFile1.read(inBuf1))
	{
		inFile2.read(inBuf2);
		outBuf.nEl = inBuf1.nEl;
		outBuf.noDataValue = inBuf1.noDataValue;
		for (int i = 0; i < inBuf1.nEl; i++)
		{
			if ((inBuf1.buf[i] != inBuf1.noDataValue) || (inBuf2.buf[i] != inBuf2.noDataValue))
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