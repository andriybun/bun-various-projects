#include <iostream>
#include <string>
#include <vector>

#include "raster.h"

using namespace std;

struct runParamsT
{
	std::string workingDir;
	std::string tmpDir;
	std::string resultDir;
};

float processRasters(float val1, float val2)
{
	return (val1 + 2) * val2;
}

float processMultipleRasters(const vector<float> & vec)
{
	float sum = 0;
	for (size_t idx = 0; idx < vec.size(); idx++)
	{
		sum += vec[idx];
	}
	return sum;
}

int main()
{
	printf(__TIME__ "\n");
	bool deleteFloats = true;
	runParamsT runParams;

	runParams.resultDir = "E:\\GIS\\cropland calibrated\\";
	runParams.tmpDir = "E:\\GIS\\cropland calibrated\\tmp\\";



	printf(__TIME__ "\n");
	return 0;
}