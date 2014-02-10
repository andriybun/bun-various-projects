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
							   void * runParams = NULL);

// Cropland algorithms:
void adjustCroplandProbabilityLayer(raster & inAreaRaster,
									raster & inCountriesRaster,
									raster & inClassRaster,
									raster & outClassRaster,
									const runParamsT & runParams,
									agreementTableT & agTable
									);
//void validateCropland(raster & inCroplandRawRaster,
//					  raster & inCroplandRaster,
//					  raster & inZoneRaster,
//					  raster & inClassRaster,
//					  raster & outCroplandRaster,
//					  raster & outMinClassRaster,
//					  raster & errorRaster,
//					  const runParamsT & runParams);
void validateCropland(raster & inCroplandRawRaster,
					  raster & inCroplandRaster,
					  raster & inZoneRaster,
					  raster & inClassRaster,
					  raster & outCroplandRaster,
					  raster & outMinClassRaster,
					  const runParamsT & runParams);
void validateCropland(raster & inCroplandRawRaster,
					  raster & inCroplandRaster,
					  raster & inZoneRaster,
					  raster & inClassRaster,
					  raster & outCroplandRaster,
					  const runParamsT & runParams);
void calibrateCropland(raster & inCroplandRawRaster,
					   raster & inCroplandRaster,
					   raster & inClassRaster,
					   raster & statisticsLevelUp,
					   raster & statisticsLevel,
					   raster & resultLevelUp,
					   raster & resultLevel,
					   raster & outCalibratedRasterLevel,
					   const runParamsT & runParams);
void calibrateCropland(raster & inCroplandRawRaster,
					   raster & inCroplandRaster,
					   raster & inClassRaster,
					   raster & statisticsLevelUp,
					   raster & statisticsLevel,
					   raster & resultLevelUp,
					   raster & resultLevel,
					   raster & outCalibratedRasterLevel,
					   raster & outMinClassRaster,
					   const runParamsT & runParams);
void validateResult(raster & areaRaster,
					raster & computedResultRaster,
					raster & statisticsRaster,
					raster & outTotalCroplandRaster,
					raster & outAbsDiffRaster,
					raster & outRelDiffRaster,
					raster & outAdjustedResultRaster,
					const runParamsT & runParams);

#endif
