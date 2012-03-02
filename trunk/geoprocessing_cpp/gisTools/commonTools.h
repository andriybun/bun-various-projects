#ifndef COMMON_TOOLS_H_
#define COMMON_TOOLS_H_

#include "raster.h"
#include "assertInt.h"
#include "errorCodes.h"

void getAreasWithCropland(raster & inZoneRaster,
						  raster & inCroplandRaster,
						  raster & isDataRaster,
						  float selectionThreshold,
						  const runParamsT & runParams);

#endif