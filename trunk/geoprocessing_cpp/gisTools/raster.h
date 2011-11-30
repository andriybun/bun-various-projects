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
#include <assert.h>

// Using python.h in raster.cpp requires setting system variable:
//   PYTHON_INCLUDE = {path to your pythonXX\include\ directory}
// and adding $(PYTHON_INCLUDE) to additional include directories in project properties
// The main project will require setting system variable:
//   PYTHON_LIB = {path to your pythonXX\libs\ directory}
// and adding $(PYTHON_LIB) to additional library dependencies in project properties

using namespace std;

#define xmin(a, b)  (((a)<(b)) ? (a) : (b))
#define xmax(a, b)  (((a)>(b)) ? (a) : (b))

const int MAX_READ_BUFFER_ELEMENTS	= 100 * 1024 * 1024;

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

	bool validateExtent(const raster & other) const;
	void saveHdr();
	bool fileExists(const string & fileName);
	void copyFile(const string & source, const string & destination) const;
	void copyProperties(raster & destination) const;
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

	// tableT class for tables
	class tableT
	{
	private:
		map< float, vector<float> > data;
		size_t numCols;
	public:
		tableT(size_t sz)
		{
			numCols = sz;
		}

		~tableT()
		{

		}

		void inc(const float key, const vector<float> & val)
		{
			assert(val.size() == numCols);
			vector<float> tmp;
			tmp = data[key];
			for (size_t idx = 0; idx < numCols; idx++)
			{
				tmp[idx] += val[idx];
			}
			data.erase(key);
			data.insert(make_pair<float, vector<float>>(key, tmp));
		}
	};
	// end of tableT class

	raster(const string & rasterName);
	raster(const raster& g);
	raster & operator = (const raster& g);
	~raster();

	// Geoprocessing methods:
	raster & copy(const string & destinationName);
	void copy(raster & destinationRaster); 
	void removeFromDisc();
	void rasterArithmetics(float (*func)(float, float), const float num, raster & outRaster);
	void rasterArithmetics(float (*func)(float, float), const raster & inRaster, raster & outRaster);
	void zonalStatistics(const raster & inZoneRaster, raster & outRaster, statisticsTypeT statisticsType = SUM);

	// Conversion
	void convertRasterToFloat();
	void convertFloatToRaster();

	friend void multipleRasterArithmetics(
		float (*func)(const vector<float> & ), 
		const vector<raster> & inRastersVector, 
		raster & outRaster);
	friend void multipleRasterArithmeticsAsTable(
		float (*func)(const vector<float> & , vector<float> & ), 
		const vector<raster> & inRastersVector, 
		raster::tableT & outTable);

};

void multipleRasterArithmetics(float (*func)(const vector<float> & ), 
							   const vector<raster> & inRastersVector, 
							   raster & outRaster);
void multipleRasterArithmeticsAsTable(float (*func)(const vector<float> & , vector<float> & ), 
									  const vector<raster> & inRastersVector, 
									  raster::tableT & outTable);

#endif
