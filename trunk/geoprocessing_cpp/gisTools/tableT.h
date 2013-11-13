#ifndef TABLET_H_
#define TABLET_H_

#include <iostream>
#include <cstdlib>
#include <map>
#include <vector>

#include "assertInt.h"
#include "errorCodes.h"
#include "rasterFriends.h"

using namespace std;

class raster;

class tableT
{
public:
	typedef map< int, vector<float> > dataT;
	dataT data;
private:
	bool sized;
	size_t numCols;
public:
	tableT();
	tableT(size_t sz);
	~tableT();
	void setNumCols(size_t n);
	void insert(const int key, const vector<float> & val);
	void inc(const int key, const vector<float> & val);
	void inc(const int key, const size_t idx, const float val);
	// TODO: optimize
	dataT::iterator find(const vector<float> & val);
	bool exists(const vector<float> & val);
	size_t size();

	// Friends:
	friend class raster;
	friend void multipleRasterArithmetics(
		float (*func)(const vector<float> & ), 
		const vector<raster *> & inRastersVector, 
		raster & outRaster);
	friend void multipleRasterArithmetics(
		void (*func)(const vector<float> &, const vector<float> &, const vector<float> &, vector<float> &),
		const vector<raster *> & inRastersVector,
		vector<raster *> & outRastersVector);
	//friend void multipleRasterArithmeticsAsTable(
	//	float (*func)(const vector<float> & , vector<float> & ), 
	//	const vector<raster *> & inRastersVector, 
	//	tableT & outTable);
	friend void validateCropland(raster & inCroplandRaster,
		raster & inZoneRaster,
		raster & inClassRaster,
		raster & outCroplandRaster,
		raster & errorRaster);
	friend void validateCropland(raster & inCroplandRaster,
		raster & inZoneRaster,
		raster & inClassRaster,
		raster & outCroplandRaster);
	friend void calibrateCropland(raster & inCroplandRaster,
		raster & inClassRaster,
		const raster & statisticsLevelUp,
		const raster & statisticsLevel,
		raster & resultLevelUp,
		raster & resultLevel,
		raster & outCalibratedRasterLevel,
		const runParamsT & params);
};

#endif