#include "BigFile.h"

//////////////////////////////////////////////////////////////////////////
// Base
//////////////////////////////////////////////////////////////////////////

BigFile::~BigFile()
{
}

template<class fStreamT>
void BigFile::openBase(const raster &r, fStreamT* file, std::ios_base::openmode mode)
{
	ASSERT_INT(!this->isOpen, FILE_ALREADY_OPEN);
	file->open(r.getFltPath().c_str(), mode | std::ios::binary);
	ASSERT_INT(file->is_open(), FILE_NOT_OPEN);	
	this->isOpen = true;
	this->numCells = r.extent.getNumCells();
	numCellsProcessed = 0;
	this->noDataValue = r.noDataValue;
}

void BigFile::printProgress()
{
	if (this->isOpen)
	{
		printf("%5.2f%% processed\n", (float)100 * this->numCellsProcessed / this->numCells);
	}
	else
	{
		printf("File not open yet\n");
	}
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
	this->isOpen = false;
	this->open(readRaster);
}

BigFileIn::~BigFileIn(void)
{
	this->file.close();
}

void BigFileIn::open(const raster &readRaster)
{
	this->openBase(readRaster, &(this->file), std::ios::in);
}

int BigFileIn::read(rasterBufT &rBuf)
{
	int bufSize = xmin(this->numCells, MAX_READ_BUFFER_ELEMENTS);
	bufSize = xmin(bufSize, this->numCells - this->numCellsProcessed);
	rBuf.buf.allocateOnce(bufSize);
	rBuf.nEl = bufSize;
	this->file.read(reinterpret_cast<char*>(rBuf.buf.ptr()), sizeof(float) * bufSize);
	this->numCellsProcessed += bufSize;
	rBuf.noDataValue = this->noDataValue;
	return bufSize;
}

float BigFileIn::read(int pos)
{
	float result;
	auto iniPos = this->file.tellg();
	this->file.seekg(pos * sizeof(float));
	this->file.read(reinterpret_cast<char*>(&result), sizeof(float));
	this->file.seekg(iniPos);
	return result;
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
	this->isOpen = false;
	this->open(writeRaster);
}

BigFileOut::~BigFileOut(void)
{
	this->file.close();
}

void BigFileOut::open(const raster &writeRaster)
{
	this->openBase(writeRaster, &(this->file), std::ios::out);
}

void BigFileOut::write(rasterBufT &rBuf)
{
	int bufSize = rBuf.nEl;
	this->file.write(reinterpret_cast<char *>(rBuf.buf.ptr()), sizeof(float) * bufSize);
	this->numCellsProcessed += bufSize;
}