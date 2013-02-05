#ifndef UTILS_H_
#define UTILS_H_

#include <map>
#include <string>
#include <vector>

#include "csv_parser.hpp"

#include "agreementTable.h"
#include "raster.h"
#include "rasterFriends.h"

using namespace std;

const int NO_VALIDATION_POINTS_COUNTRY_ID = -1111;

//struct priorityDataT
//{
//	vector<int> prioritiesVector;
//	vector<int> prioritiesVector2;
//	vector<int> weightsVector;
//	agreementTableT * agTable;
//};

int initializePriorityData(string csvPriorityFile, int numRasters, map<int, priorityDataT * > &priorityDataMap);
int destroyPriorityData(map<int, priorityDataT * > &priorityDataMap);

void adjustCroplandProbabilityLayer(raster & inAreaRaster,
									raster & inCountriesRaster,
									raster & inClassRaster,
									raster & outClassRaster,
									const runParamsT & params,
									map<int, priorityDataT * > & priorityDataMap
									);

#endif