#ifndef GIS_TOOLS_INTERFACE_H_
#define GIS_TOOLS_INTERFACE_H_

#include "stdafx.h"

extern "C" __declspec(dllexport) int checkZonesForData(
	int numRasters,
	const char * listOfRasterPaths [],
	const char * zoneRasterPath,
	//const char * resultDir,
	const char * tmpDir,
	float selectionThreshold
	);



#endif