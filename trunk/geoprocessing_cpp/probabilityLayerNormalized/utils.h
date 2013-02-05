#ifndef UTILS_H_
#define UTILS_H_

#include <vector>
#include "agreementTable.h"
#include "raster.h"

using namespace std;

//struct priorityDataT
//{
//	vector<int> prioritiesVector;
//	vector<int> prioritiesVector2;
//	vector<int> weightsVector;
//	agreementTableT * agTable;
//};

int initializePriorityData(char * argv[], int numRasters, priorityDataT * &priorityData);

#endif