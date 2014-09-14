#pragma once

#include <fstream>
#include <string>

#include "assertInt.h"
#include "raster.h"

//const int MAX_READ_BUFFER_ELEMENTS	= 40 * 1024 * 1024;

class raster;

struct rasterBufT
{
	float* buf;
	float noDataValue;
	int nEl;
};

class BigFile
{
protected:
	bool isOpen;
	int numCells;
	int numCellsProcessed;
	float noDataValue;
	float* buf;

	template<class fStreamT>
	void openBase(const raster &r, fStreamT* file, std::ios_base::openmode mode);
public:
	BigFile(void) {};
	virtual ~BigFile(void) = 0;
	virtual void open(const raster &r) = 0;
};

class BigFileIn : public BigFile
{
	std::ifstream file;
public:
	BigFileIn(void);
	BigFileIn(const raster &readRaster);
	~BigFileIn(void);
	void open(const raster &readRaster);
	int read(rasterBufT &rBuf);
};

class BigFileOut : public BigFile
{
	std::ofstream file;
public:
	BigFileOut(void);
	BigFileOut(const raster &writeRaster);
	~BigFileOut(void);
	void open(const raster &writeRaster);
	int write(rasterBufT &rBuf);
};