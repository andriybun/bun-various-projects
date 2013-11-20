//   Name:          Raster class for geoprocessing dadta in ESRI binary grid format
//   Author:        Andriy Bun
//   Date:          09.12.2011

// TODO: introduce functions/methods accounting for nodata
// TODO: improve logging
// TODO: put float grids to temporary folder

#ifndef raster_h_
#define raster_h_

#include <iostream>
#include <cstdlib>
#include <string>
#include <map>
#include <cmath>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <typeinfo>

#include <windows.h>

#include "assertInt.h"
#include "errorCodes.h"
#include "tableT.h"
//#include "rasterFriends.h"


// Using python.h in raster.cpp requires setting system variable:
//   PYTHON_INCLUDE = {path to your pythonXX\include\ directory}
// and adding $(PYTHON_INCLUDE) to additional include directories in project properties
// The main project will require setting system variable:
//   PYTHON_LIB = {path to your pythonXX\libs\ directory}
// and adding $(PYTHON_LIB) to additional library dependencies in project properties

using namespace std;

#define xmin(a, b)					(((a)<(b)) ? (a) : (b))
#define xmax(a, b)					(((a)>(b)) ? (a) : (b))
#define compare_eq(a, b, epsilon)	((fabs(float(a)-float(b)) < epsilon) ? true : false)

const int MAX_READ_BUFFER_ELEMENTS	= 40 * 1024 * 1024;
const double EPSILON				= 1e-6;

// Simple arithmetic operations functions to use with raster arithmetics:
float xplus(float val1, float val2);
float xminus(float val1, float val2);
float xtimes(float val1, float val2);
float xdivide(float val1, float val2);

struct priorityDataT
{
	vector<int> prioritiesVector;
	vector<int> prioritiesVector2;
	vector<int> weightsVector;
	agreementTableT * agTable;
};

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
		EMPTY,
		PASS_INPUT,
		PASS_TEMPORARY
	};
	typedef map<int, statisticsStructT> zonalStatisticsTableT;
private:
	struct unitResultT
	{
		int bestClass;
		float bestEstimate;
		float bestClassMultiplier;
		float error;
	};
	typedef map <int, unitResultT> summaryTableT;

	string rasterName;
	string rasterPath;
	int horResolution;
	int verResolution;
	double xMin;
	double yMin;
	double cellSize;
	float noDataValue;
	bool isDescribed;
	statisticsStructT description;
	bool initializedFromImg;
	rasterTypeT rasterType;

	void rasterInitPrivate(const string & rasterName, rasterTypeT rType);
	bool readRasterProperties();
	bool validateExtent(const raster & other) const;
	void saveHdr();
	bool fileExists(const string & fileName);
	void copyFile(const string & source, const string & destination) const;
	void deleteFile(const string & fileName) const;
	void copyProperties(raster & destination) const;
	void incMap(zonalStatisticsTableT &mp, int key, float val);
public:
	raster();
	raster(const string & rasterName, rasterTypeT rType);
	void rasterInit(const string & rasterName, rasterTypeT rType);
	raster(const raster & g);
	raster & operator = (const raster & g);
	~raster();

	bool isEmpty();

	// Generic geoprocessing methods:
	raster copy(const string & destinationName);
	void copy(raster & destinationRaster); 
	void removeFloatFromDisc();
	void rasterArithmetics(float (*func)(float, float), const float num, raster & outRaster);
	void rasterArithmetics(float (*func)(float, float), const raster & inRaster, raster & outRaster);
	void zonalStatisticsAsTable(const raster & inZoneRaster,
		zonalStatisticsTableT & zonalStatisticsTable,
		statisticsTypeT statisticsType = SUM);
	void zonalStatistics(const raster & inZoneRaster, raster & outRaster, statisticsTypeT statisticsType = SUM);
	void combineAsTable(const vector<raster *> & inRastersVector, tableT & outTable);
	statisticsStructT describe();

	// Some cropland specific methods
	void zonalSumByClassAsTable(const raster & inZoneRaster,
		raster & inClassRaster,
		summaryTableT & calibratedResults,
		const runParamsT & runParams);

	// Conversion
	void convertRasterToFloat();
	void convertFloatToRaster();

	// Paths to raster files
	string getHdrPath();
	string getFltPath();

	friend void multipleRasterArithmetics(
		float (*func)(const vector<float> & ), 
		const vector<raster *> & inRastersVector, 
		raster & outRaster);
	friend void multipleRasterArithmetics(
		void (*func)(const vector<float> &, const vector<float> &, const vector<float> &, vector<float> &),
		const vector<raster *> & inRastersVector,
		vector<raster *> & outRastersVector);
	friend void multipleRasterArithmetics(
		void (*func)(const vector<float> &, const vector<float> &, const vector<float> &, vector<float> &, void *),
		const vector<raster *> & inRastersVector,
		vector<raster *> & outRastersVector,
		void * runParams);
	friend void adjustCroplandProbabilityLayer(raster & inAreaRaster,
		raster & inCountriesRaster,
		raster & inClassRaster,
		raster & outClassRaster,
		const runParamsT & runParams,
		agreementTableT & agTable
		);
	friend void adjustCroplandProbabilityLayer(raster & inAreaRaster,
		raster & inCountriesRaster,
		raster & inClassRaster,
		raster & outClassRaster,
		const runParamsT & runParams,
		map<int, priorityDataT * > & priorityDataMap
		);
	friend void validateCropland(
		raster & inCroplandRawRaster,
		raster & inCroplandRaster,
		raster & inZoneRaster,
		raster & inClassRaster,
		raster & outCroplandRaster,
		const runParamsT & runParams);
	friend void validateCropland(
		raster & inCroplandRawRaster,
		raster & inCroplandRaster,
		raster & inZoneRaster,
		raster & inClassRaster,
		raster & outCroplandRaster,
		raster & outMinClassRaster,
		const runParamsT & runParams);
	friend void validateCropland(
		raster & inCroplandRawRaster,
		raster & inCroplandRaster,
		raster & inZoneRaster,
		raster & inClassRaster,
		raster & outCroplandRaster,
		raster & outMinClassRaster,
		raster & errorRaster,
		const runParamsT & runParams);
	friend void calibrateCropland(
		raster & inCroplandRawRaster,
		raster & inCroplandRaster,
		raster & inClassRaster,
		raster & statisticsLevelUp,
		raster & statisticsLevel,
		raster & resultLevelUp,
		raster & resultLevel,
		raster & outCalibratedRasterLevel,
		const runParamsT & runParams);
	friend void calibrateCropland(
		raster & inCroplandRawRaster,
		raster & inCroplandRaster,
		raster & inClassRaster,
		raster & statisticsLevelUp,
		raster & statisticsLevel,
		raster & resultLevelUp,
		raster & resultLevel,
		raster & outCalibratedRasterLevel,
		raster & outMinClassRaster,
		const runParamsT & runParams);
	friend void validateResult(raster & cellAreaStatRaster,
		raster & statisticsRaster,
		raster & outAbsDiffRaster,
		raster & outRelDiffRaster,
		const runParamsT & runParams);

};

#endif
