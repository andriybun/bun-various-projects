#include <iostream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>

#include "raster.h"
#include "SpatialAnalyst.h"
#include "time_tools.h"

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

void processMultipleRasters(const vector<float> & vec,
							const vector<float> & noDataInVec,
							const vector<float> & noDataOutVec,
							vector<float> & outVec)
{
	size_t nEl = vec.size();
	for (size_t idx = 0; idx < nEl; idx++)
	{
		if (vec[idx] != noDataInVec[idx])
		{
			outVec[0] += vec[idx];
		}
		else
		{
			outVec[0] = noDataOutVec[0];
			return;
		}
	}
}

namespace po = boost::program_options;

po::options_description DefineOptions()
{
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("in_raster_1", po::value<string>(), "first input raster")
		("in_raster_2", po::value<string>(), "second input raster")
		("out_raster", po::value<string>(), "output raster")
		;
	return desc;
}

int main(int argc, char* argv[])
{
	cout << getTime() << endl;
	bool deleteFloats = true;
	runParamsT runParams;

	//runParams.resultDir = "E:\\GIS\\cropland calibrated\\";
	//runParams.tmpDir = "E:\\GIS\\cropland calibrated\\tmp\\";

	auto desc = DefineOptions();
	po::variables_map vm;  
	try
	{
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);
	}
	catch (std::exception& e)
	{
		cout << "There was a problem with parsing command line parameters." << endl;
		e.what();
		return INCORRECT_INPUT_PARAMS;
	}

	if (vm.count("help"))
	{
		cout << desc << "\n";
		return 0;
	}
	int nRasters = 0;
	string inRasterPath1, inRasterPath2, outRasterPath; 
	if (vm.count("in_raster_1"))
	{
		inRasterPath1 = vm["in_raster_1"].as<string>();
		nRasters++;
	}
	if (vm.count("in_raster_2"))
	{
		inRasterPath2 = vm["in_raster_2"].as<string>();
		nRasters++;
	}
	if (vm.count("out_raster"))
	{
		outRasterPath = vm["out_raster"].as<string>();
		nRasters++;
	}

	ASSERT_INT(nRasters == 3, INCORRECT_INPUT_PARAMS);
	raster inRaster1(inRasterPath1, raster::INPUT);
	raster inRaster2(inRasterPath2, raster::INPUT);
	raster outRaster(outRasterPath, raster::OUTPUT);

	//SpatialAnalyst::RasterArithmetics(
	//	&processRasters, 
	//	inRaster1, 
	//	inRaster2, 
	//	outRaster);

	//// TODO: fix this:
	//vector<raster *> inRasterVec, outRasterVec;
	//inRasterVec.push_back(&inRaster1);
	//inRasterVec.push_back(&inRaster2);
	//raster outRaster2(outRasterPath + "_multi", raster::OUTPUT);
	//outRasterVec.push_back(&outRaster2);
	//SpatialAnalyst::MultipleRasterArithmetics(&processMultipleRasters, inRasterVec, outRasterVec);

	SpatialAnalyst::ZonalStatistics(inRaster1, inRaster2, outRaster, SpatialAnalyst::SUM);

	cout << getTime() << endl;
	return 0;
}