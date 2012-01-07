#ifndef RASTER_FRIENDS_H_
#define RASTER_FRIENDS_H_

#include <vector>

using namespace std;

#include "assertInt.h"
#include "errorCodes.h"

class raster;
class tableT;

struct runParamsT
{
	string workingDir;
	string tmpDir;
	string resultDir;
};

void multipleRasterArithmetics(float (*func)(const vector<float> & ), 
							   const vector<raster *> & inRastersVector, 
							   raster & outRaster);
void multipleRasterArithmetics(void (*func)(const vector<float> &, const vector<float> &, const vector<float> &, vector<float> &),
							   const vector<raster *> & inRastersVector,
							   vector<raster *> & outRastersVector);
void multipleRasterArithmetics(void (*func)(const vector<float> &, const vector<float> &, const vector<float> &, vector<float> &, void *),
							   const vector<raster *> & inRastersVector,
							   vector<raster *> & outRastersVector,
							   void * params = NULL);
//void multipleRasterArithmeticsAsTable(float (*func)(const vector<float> & , vector<float> & ), 
//									  const vector<raster *> & inRastersVector, 
//									  tableT & outTable);

// Cropland algorithms:
void validateCropland(raster & inCroplandRaster,
					  raster & inZoneRaster,
					  raster & inClassRaster,
					  raster & outCroplandRaster,
					  raster & errorRaster);
void validateCropland(raster & inCroplandRaster,
					  raster & inZoneRaster,
					  raster & inClassRaster,
					  raster & outCroplandRaster);
void calibrateCropland(raster & inCroplandRaster,
					   raster & inClassRaster,
					   raster & statisticsLevelUp,
					   raster & statisticsLevel,
					   raster & resultLevelUp,
					   raster & resultLevel,
					   raster & outCalibratedRasterLevel,
					   const runParamsT & params);
void validateResult(raster & cellAreaStatRaster,
					raster & statisticsRaster,
					raster & outAbsDiffRaster,
					raster & outRelDiffRaster,
					const runParamsT & runParams);

#endif
