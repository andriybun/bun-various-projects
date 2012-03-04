#include "stdafx.h"

#include <stdexcept>

#include "raster.h"
#include "commonTools.h"
#include "assertInt.h"
#include "errorCodes.h"

#include "gisToolsInterface.h"

using namespace std;

int checkZonesForData(
	int numRasters,
	const char * listOfRasterPaths [],
	const char * zoneRasterPath,
	//const char * resultDir,
	const char * tmpDir,
	float selectionThreshold
	)
{
	runParamsT runParams;
	//runParams.resultDir = string(resultDir) + "\\";
	runParams.tmpDir = string(tmpDir) + "\\";
	raster zoneRaster(zoneRasterPath, raster::INPUT);
	
	for (int idx = 0; idx < numRasters; idx++)
	{
		raster croplandRaster(listOfRasterPaths[idx], raster::INPUT);
		string tmpName = string(listOfRasterPaths[idx]) + "_is_data";
		raster isDataRaster(tmpName, raster::OUTPUT);
		getAreasWithCropland(zoneRaster, croplandRaster, isDataRaster, selectionThreshold, runParams);
	}

	return 0;
}