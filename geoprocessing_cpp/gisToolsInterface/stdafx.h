// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <iostream>
#include <string>
#include <vector>

extern "C" __declspec(dllexport) int checkZonesForData(
	int numRasters,
	const char * listOfRasterPaths [],
	const char * zoneRasterPath,
	//const char * resultDir,
	const char * tmpDir,
	float selectionThreshold
	);

