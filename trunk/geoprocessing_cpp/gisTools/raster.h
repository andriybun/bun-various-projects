//   Name:          GridData class 2 + any type
//   Author:        Andriy Bun; Mykola Gusti
//   Date:          26.08.2009

#ifndef raster_h_
#define raster_h_

#include <iostream>
#include <cstdlib>
#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <typeinfo>

#include <windows.h>

#include <assert.h>

using namespace std;

#define xmin(a, b)  (((a)<(b)) ? (a) : (b))
#define xmax(a, b)  (((a)>(b)) ? (a) : (b))

const int MAX_READ_BUFFER_ELEMENTS	= 100 * 1024 * 1024;
const int MAX_READ_BUFFER_SIZE		= MAX_READ_BUFFER_ELEMENTS * 8;

//template <class T>
class raster
{
private:
	struct statisticsStructT
	{
		float sumVal;
		float minVal;
		float maxVal;
		int count;
	};
	string rasterPath;
	int horResolution;
	int verResolution;
	double xMin;
	double yMin;
	double cellSize;
	float noDataValue;

	bool validateExtent(const raster & other);
	void saveHdr();
	void copyFile(const string & source, const string & destination);
	//void incMap(map<float, float> &mp, float key, float val);
	//void incMap(map<float, int> &mp, float key, float val);
	void incMap(map<float, statisticsStructT> &mp, float key, float val);
public:
	enum statisticsTypeT
	{
		SUM,
		MEAN,
		MIN,
		MAX,
		COUNT
	};

	raster(const string & rasterName);
	raster(const raster& g);
	raster & operator = (const raster& g);
	~raster();

	// Geoprocessing methods:
	raster & copy(const string & destinationName);
	void copy(const raster & destinationRaster); 
	void removeFromDisc();
	void rasterArithmetics(float (*func)(float, float), const float num, const raster & outRaster);
	void rasterArithmetics(float (*func)(float, float), const raster & inRaster, const raster & outRaster);
	void zonalStatistics(const raster & inZoneRaster, const raster & outRaster, statisticsTypeT statisticsType = SUM);

	//friend void plus(const raster & first, const float num/*const raster & second*/, raster &result);
};

//void plus(const raster & first, const float num/*const raster & second*/, raster &result);

#endif
