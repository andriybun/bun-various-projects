#pragma once

#include <cstdio>
#include <fstream>
#include <cmath>
#include <string>

#include "assertInt.h"
#include "errorCodes.h"

const double EPSILON				= 1e-6;

inline bool compare_eq(double a, double b, double epsilon)
{
	return ((fabs(double(a)-double(b)) < epsilon) ? true : false);
}

// Extent class
// Handles rasters' extents
class extentT
{
private:
	bool isInitialized;
	int horResolution;
	int verResolution;
	double xMin;
	double yMin;
	double cellSize;
public:
	extentT()
	{
		this->isInitialized = false;
	}

	extentT(int horResolution, int verResolution, double xMin, double yMin, double cellSize)
	{
		this->init(horResolution, verResolution, xMin, yMin, cellSize);
	}

	void init(int horResolution, int verResolution, double xMin, double yMin, double cellSize)
	{
		this->horResolution = horResolution;
		this->verResolution = verResolution;
		this->xMin = xMin;
		this->yMin = yMin;
		this->cellSize = cellSize;
		this->isInitialized = true;
	}

	bool operator == (const extentT &other) const
	{
		return isExtentEqual(other);
	}

	bool operator != (const extentT &other) const
	{
		return !(*this == other);
	}

	bool isExtentEqual(const extentT &other) const 
	{
		ASSERT_INT(this->isInitialized, OTHER_ERROR);
		bool result = true;
		if (this->horResolution != other.horResolution)
		{
			result = false;
			printf("hor resolution: %d - %d\n", this->horResolution, other.horResolution);
		}
		if (this->verResolution != other.verResolution)
		{
			result = false;
			printf("ver resolution: %d - %d\n", this->verResolution, other.verResolution);
		}
		if (!compare_eq(this->xMin, other.xMin, EPSILON))
		{
			result = false;
			printf("x min:          %f - %f\n", this->xMin, other.xMin);
		}
		if (!compare_eq(this->yMin, other.yMin, EPSILON))
		{
			result = false;
			printf("y min:          %f - %f\n", this->yMin, other.yMin);
		}
		if (!compare_eq(this->cellSize, other.cellSize, EPSILON))
		{
			result = false;
			printf("cell size:      %f - %f\n", this->cellSize, other.cellSize);
		}
		return result;
	}

	void saveHdr(std::string rasterPath, float noDataValue)
	{
		ASSERT_INT(this->isInitialized, OTHER_ERROR);
		std::ofstream file;
		file.open((rasterPath + ".hdr").c_str(), std::ios::out);
		file << "ncols         " << horResolution << std::endl;
		file << "nrows         " << verResolution << std::endl;
		file << "xllcorner     " << xMin << std::endl;
		file << "yllcorner     " << yMin << std::endl;
		file << "cellsize      " << cellSize << std::endl;
		file << "NODATA_value  " << noDataValue << std::endl;
		file << "byteorder     LSBFIRST" << std::endl;
		file.close();
	}

	int getNumCells() const
	{
		ASSERT_INT(this->isInitialized, OTHER_ERROR);
		return this->horResolution * this->verResolution;
	}
};
