#include <iostream>
#include <string>
#include <boost/program_options.hpp>

#include "raster.h"
#include "SpatialAnalyst.h"
#include "time_tools.h"

using namespace std;
namespace po = boost::program_options;

struct runParamsT
{
	string workingDir;
	string tmpDir;
	string resultDir;
};


po::options_description DefineOptions()
{
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
#OPTIONS_DESCRIPTION
		;
	return desc;
}

int main(int argc, char * argv[])
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

	int nParams = 0;
#PARSE_PARAMS_DECLARATION
#PARSE_PARAMS
	ASSERT_INT(nParams == #NUM_PARAMS, INCORRECT_INPUT_PARAMS);

	/////////////////////////////////////////////////////////
	//                          
	//  Insert your code here   
	//                          
	/////////////////////////////////////////////////////////
	// 
	// Initialize rasters as follows:
	//	raster inRaster(inRasterPath, raster::INPUT);
	//	raster outRaster(outRasterPath, raster::OUTPUT);
	// Raster paths may be obtained from command line parameters
	//
	/////////////////////////////////////////////////////////

	cout << getTime() << endl;
	return 0;
}