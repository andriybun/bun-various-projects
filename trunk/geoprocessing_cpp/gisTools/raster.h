//   Name:          GridData class 2 + any type
//   Author:        Andriy Bun; Mykola Gusti
//   Date:          26.08.2009

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

#define ASSERT_INT(ARG)														\
	{																		\
		if (!(ARG))															\
		{																	\
			fprintf(stderr, __TIME__ "\n");									\
			fprintf(stderr, "File: " __FILE__ " \tline: %d\n", __LINE__);	\
			fprintf(stderr, "Assertion failed: " #ARG "\n");				\
			exit(1);														\
		}																	\
	}

// Using python.h in raster.cpp requires setting system variable:
//   PYTHON_INCLUDE = {path to your pythonXX\include\ directory}
// and adding $(PYTHON_INCLUDE) to additional include directories in project properties
// The main project will require setting system variable:
//   PYTHON_LIB = {path to your pythonXX\libs\ directory}
// and adding $(PYTHON_LIB) to additional library dependencies in project properties

using namespace std;

#define xmin(a, b)  (((a)<(b)) ? (a) : (b))
#define xmax(a, b)  (((a)>(b)) ? (a) : (b))

const int MAX_READ_BUFFER_ELEMENTS	= 40 * 1024 * 1024;

struct runParamsT
{
	string tmpDir;
	string resultDir;
};

class raster
{
public:
	// Structure with statistics
	struct statisticsStructT
	{
		float sumVal;
		float minVal;
		float maxVal;
		float meanVal;
		int count;
	};
	typedef map<float, statisticsStructT> zonalStatisticsTableT;
private:
	struct unitResultT
	{
		int bestClass;
		float bestEstimate;
		float error;
	};
	typedef map <float, unitResultT> summaryTableT;

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
	bool isTmp;

	bool readRasterProperties();
	bool validateExtent(const raster & other) const;
	void saveHdr();
	bool fileExists(const string & fileName);
	void copyFile(const string & source, const string & destination) const;
	void copyProperties(raster & destination) const;
	void incMap(zonalStatisticsTableT &mp, float key, float val);
public:
	// List of possible statistics to compute
	enum statisticsTypeT
	{
		SUM,
		MEAN,
		MIN,
		MAX,
		COUNT
	};
	// tableT class for tables
	class tableT
	{
	public:
		typedef map< float, vector<float> > dataT;
	private:
		dataT data;
		bool sized;
		size_t numCols;
	public:
		tableT();
		tableT(size_t sz);
		~tableT();
		void setNumCols(size_t n);
		void inc(const float key, const vector<float> & val);
		void inc(const float key, const size_t idx, const float val);
		size_t size();
		friend class raster;
	};
	// end of tableT class

	raster(const string & rasterName, bool isTemporary = false);
	raster(const raster & g);
	raster & operator = (const raster & g);
	~raster();

	// Generic geoprocessing methods:
	raster copy(const string & destinationName);
	void copy(raster & destinationRaster); 
	void removeFloatFromDisc();
	void rasterArithmetics(float (*func)(float, float), const float num, raster & outRaster);
	void rasterArithmetics(float (*func)(float, float), const raster & inRaster, raster & outRaster);
	void zonalStatisticsAsTable(const raster & inZoneRaster, zonalStatisticsTableT & zonalStatisticsTable, statisticsTypeT statisticsType = SUM);
	void zonalStatistics(const raster & inZoneRaster, raster & outRaster, statisticsTypeT statisticsType = SUM);
	statisticsStructT describe();

	// Some specific methods
	void zonalSumByClassAsTable(const raster & inZoneRaster,
		raster & inClassRaster,
		summaryTableT & calibratedResults);

	// Conversion
	void convertRasterToFloat();
	void convertFloatToRaster();

	friend void multipleRasterArithmetics(
		float (*func)(const vector<float> & ), 
		const vector<raster> & inRastersVector, 
		raster & outRaster);
	//friend void multipleRasterArithmeticsAsTable(
	//	float (*func)(const vector<float> & , vector<float> & ), 
	//	const vector<raster> & inRastersVector, 
	//	raster::tableT & outTable);
	friend void validateCropland(raster & inCroplandRaster,
		raster & inZoneRaster,
		raster & inClassRaster,
		raster & outCroplandRaster,
		raster & errorRaster);
	friend void calibrateCropland(const raster & statisticsLevelUp,
		const raster & statisticsLevel,
		raster & resultLevelUp,
		raster & resultLevel,
		raster & calibratedResultLevel,
		const runParamsT & params);

};

void multipleRasterArithmetics(float (*func)(const vector<float> & ), 
							   const vector<raster> & inRastersVector, 
							   raster & outRaster);
//void multipleRasterArithmeticsAsTable(float (*func)(const vector<float> & , vector<float> & ), 
//									  const vector<raster> & inRastersVector, 
//									  raster::tableT & outTable);
void validateCropland(raster & inCroplandRaster,
					  raster & inZoneRaster,
					  raster & inClassRaster,
					  raster & outCroplandRaster,
					  raster & errorRaster);
void calibrateCropland(const raster & statisticsLevelUp,
					   const raster & statisticsLevel,
					   raster & resultLevelUp,
					   raster & resultLevel,
					   raster & calibratedResultLevel,
					   const runParamsT & params);

#endif
