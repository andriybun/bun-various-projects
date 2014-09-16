//   Name:          Raster class for geoprocessing data in ESRI binary grid format
//   Author:        Andriy Bun
//   Date:          XX.XX.2014

#pragma once

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>

#include <windows.h>

#include "assertInt.h"
#include "BigFile.h"
#include "errorCodes.h"
#include "tableT.h"
#include "extentT.h"
#include "timer.h"

const int MAX_READ_BUFFER_ELEMENTS	= 40 * 1024 * 1024;

template <class T>
inline T xmin(T a, T b)
{
	return (((a)<(b)) ? (a) : (b));
}

template <class T>
inline T xmax(T a, T b)
{
	return (((a)>(b)) ? (a) : (b));
}

// Simple arithmetic operations functions to use with raster arithmetics:
float xplus(float val1, float val2);
float xminus(float val1, float val2);
float xtimes(float val1, float val2);
float xdivide(float val1, float val2);

class BigFile;
class SpatialAnalyst;

class raster
{
public:
	struct pointDataT
	{
		bool isData;
		float value;
	};

	// Structure with statistics
	struct statisticsStructT
	{
		float sumVal;
		float minVal;
		float maxVal;
		float meanVal;
		int count;
		int countNonZero;
	};

	// List of possible statistics to compute
	enum statisticsTypeT
	{
		SUM,
		MEAN,
		MIN,
		MAX,
		COUNT,
		COUNT_NON_ZERO
	};

	// Type of raster usage
	enum rasterTypeT
	{
		INPUT,
		OUTPUT,
		TEMPORARY,
		COPY,
		EMPTY
	};

private:
	// Initialization properties
	std::string rasterName;
	std::string rasterPath;
	bool initializedFromImg;
	rasterTypeT rasterType;

	// Raster geometry
	extentT extent;
	float noDataValue;

	// Runtime properties
	bool isDescribed;
	statisticsStructT description;

	void rasterInitPrivate(const std::string & rasterName, rasterTypeT rType);
	bool readRasterProperties();
	bool validateExtent(const extentT &otherExtent) const;
	void saveHdr();
	bool fileExists(const std::string & fileName);
	void deleteFile(const std::string & fileName) const;
	void copyProperties(raster & destination) const;

	static void copyFile(const std::string & source, const std::string & destination);
public:
	raster();
	raster(const std::string & rasterName, rasterTypeT rType);
	void rasterInit(const std::string & rasterName, rasterTypeT rType);
	raster(const raster & g);
	raster & operator = (const raster & g);
	~raster();

	bool isEmpty();

	// Generic geoprocessing methods:
	raster copy(const std::string & destinationName);
	void copy(raster & destinationRaster); 
	void removeFloatFromDisc();
	//void rasterArithmetics(float (*func)(float, float), const float num, raster & outRaster);
	//void rasterArithmetics(float (*func)(float, float), const raster & inRaster, raster & outRaster);
	
	statisticsStructT describe();

	// Conversion
	void convertRasterToFloat();
	void convertFloatToRaster();

	// Paths to raster files
	std::string getHdrPath() const;
	std::string getFltPath() const;

	friend class BigFile;
	friend class SpatialAnalyst;
};
