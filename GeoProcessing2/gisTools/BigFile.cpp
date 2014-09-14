#include "BigFile.h"

//////////////////////////////////////////////////////////////////////////
// Base
//////////////////////////////////////////////////////////////////////////

template<class fStreamT>
void BigFile::openBase(const raster &r, fStreamT* file, std::ios_base::openmode mode)
{
	ASSERT_INT(!this->isOpen, FILE_ALREADY_OPEN);
	file->open(r.getFltPath().c_str(), mode | std::ios::binary);
	ASSERT_INT(file->is_open(), FILE_NOT_OPEN);	
	this->isOpen = true;
	this->numCells = r.extent.getNumCells();
	numCellsProcessed = 0;
	this->buf = new float[MAX_READ_BUFFER_ELEMENTS];
	this->noDataValue = r.noDataValue;
}

//////////////////////////////////////////////////////////////////////////
// Input file
//////////////////////////////////////////////////////////////////////////

BigFileIn::BigFileIn(void)
{
	this->isOpen = false;
	this->numCells = -1;
}

BigFileIn::BigFileIn(const raster &readRaster)
{
	this->open(readRaster);
}

BigFileIn::~BigFileIn(void)
{
	this->file.close();
	delete [] buf;
}

void BigFileIn::open(const raster &readRaster)
{
	this->openBase(readRaster, &(this->file), std::ios::in);
}

int BigFileIn::read(rasterBufT &rBuf)
{
	int bufSize = xmin(this->numCells, MAX_READ_BUFFER_ELEMENTS);
	bufSize = xmin(bufSize, this->numCells - this->numCellsProcessed);
	this->file.read(reinterpret_cast<char*>(buf), sizeof(float) * bufSize);
	rBuf.buf = buf;
	rBuf.nEl = bufSize;
	rBuf.noDataValue = this->noDataValue;
	return bufSize;
}

//////////////////////////////////////////////////////////////////////////
// Output file
//////////////////////////////////////////////////////////////////////////

BigFileOut::BigFileOut(void)
{
	this->isOpen = false;
	this->numCells = -1;
}

BigFileOut::BigFileOut(const raster &writeRaster)
{
	this->open(writeRaster);
}

BigFileOut::~BigFileOut(void)
{
	this->file.close();
	delete [] buf;
}

void BigFileOut::open(const raster &writeRaster)
{
	this->openBase(writeRaster, &(this->file), std::ios::out);
}

int BigFileOut::write(rasterBufT &rBuf)
{
	int bufSize = xmin(this->numCells, MAX_READ_BUFFER_ELEMENTS);
	bufSize = xmin(bufSize, this->numCells - this->numCellsProcessed);
	this->file.write(reinterpret_cast<char *>(buf), sizeof(float) * bufSize);
	rBuf.buf = buf;
	rBuf.nEl = bufSize;
	rBuf.noDataValue = this->noDataValue;
	return bufSize;
}

/*
{
	std::string fltPath = this->rasterPath + ".flt";
	
	std::ifstream thisFile;
	thisFile.open(fltPath.c_str(), std::ios::out | std::ios::binary);
	ASSERT_INT(thisFile.is_open(), FILE_NOT_OPEN);

	int numCells = this->extent.getNumCells();
	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);
	float * buf = new float[bufSize];

	int numCellsProcessed = 0;
	bool minMaxInitialized = false;
	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		thisFile.read(reinterpret_cast<char*>(buf), sizeof(float) * bufSize);
		for (int i = 0; i < bufSize; i++)
		{
			if (buf[i] != this->noDataValue)
			{
				if (!minMaxInitialized)
				{
					result.maxVal = buf[i];
					result.minVal = buf[i];
					minMaxInitialized = true;
				}
				result.count++;
				result.sumVal += buf[i];
				result.maxVal = xmax(buf[i], result.maxVal);
				result.minVal = xmin(buf[i], result.minVal);
			}
		}
		numCellsProcessed += bufSize;
	}

	result.meanVal = result.sumVal / result.count;

	delete [] buf;
	thisFile.close();

}
*/