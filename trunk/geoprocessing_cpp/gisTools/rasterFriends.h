#ifndef RASTER_FRIENDS_H_
#define RASTER_FRIENDS_H_

#include <vector>

using namespace std;

#include "assertInt.h"
#include "errorCodes.h"

#include "agreementTable.h"

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

// Cropland algorithms:
void adjustCroplandProbabilityLayer(raster & inAreaRaster,
									raster & inCountriesRaster,
									raster & inClassRaster,
									raster & outClassRaster,
									const runParamsT & params,
									agreementTableT & agTable
									);
void validateCropland(raster & inCroplandRawRaster,
					  raster & inCroplandRaster,
					  raster & inZoneRaster,
					  raster & inClassRaster,
					  raster & outCroplandRaster,
					  raster & outMinClassRaster,
					  raster & errorRaster);
void validateCropland(raster & inCroplandRawRaster,
					  raster & inCroplandRaster,
					  raster & inZoneRaster,
					  raster & inClassRaster,
					  raster & outCroplandRaster,
					  raster & outMinClassRaster);
void validateCropland(raster & inCroplandRawRaster,
					  raster & inCroplandRaster,
					  raster & inZoneRaster,
					  raster & inClassRaster,
					  raster & outCroplandRaster);
void calibrateCropland(raster & inCroplandRawRaster,
					   raster & inCroplandRaster,
					   raster & inClassRaster,
					   raster & statisticsLevelUp,
					   raster & statisticsLevel,
					   raster & resultLevelUp,
					   raster & resultLevel,
					   raster & outCalibratedRasterLevel,
					   const runParamsT & params);
void calibrateCropland(raster & inCroplandRawRaster,
					   raster & inCroplandRaster,
					   raster & inClassRaster,
					   raster & statisticsLevelUp,
					   raster & statisticsLevel,
					   raster & resultLevelUp,
					   raster & resultLevel,
					   raster & outCalibratedRasterLevel,
					   raster & outMinClassRaster,
					   const runParamsT & params);
void validateResult(raster & areaRaster,
					raster & computedResultRaster,
					raster & statisticsRaster,
					//raster & outNormalizedRasterLevel2,
					raster & outAbsDiffRaster,
					raster & outRelDiffRaster,
					const runParamsT & runParams);

#endif
