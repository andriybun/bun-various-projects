#include <iostream>
#include <string>
#include <boost/program_options.hpp>

#include "raster.h"
#include "SpatialAnalyst.h"
#include "time_tools.h"
#include "utils.h"

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
		("workingDir", po::value<string>(), "working directory (location of script)")
		("resultDir",  po::value<string>(), "directory where the result will be located (if aplicable)")
		("tmpDir",     po::value<string>(), "directory for storage of temporary files")
		("inCountryRaster", po::value<string>(), "ADD DESCRIPTION")
		("inValueRasterList", po::value<string>(), "ADD DESCRIPTION")
		("outDir", po::value<string>(), "ADD DESCRIPTION")

		;
	return desc;
}

void noDataZones(
	const vector<float> & vec,
	const vector<float> & noDataInVec,
	const vector<float> & noDataOutVec,
	vector<float> & outVec)
{
	if (vec[1] > 0)
	{
		outVec[0] = vec[0];
	}
	else
	{
		outVec[0] = noDataOutVec[0];
	}
}

int main(int argc, char * argv[])
{
	cout << getTime() << endl;
	bool deleteFloats = true;
	runParamsT runParams;

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
	string workingDir, resultDir, tmpDir;
	string inCountryPath;
	string inValuePathListRaw;
	string outDir;

	if (vm.count("workingDir"))
	{
		workingDir = vm["workingDir"].as<string>();
	}
	if (vm.count("resultDir"))
	{
		resultDir = vm["resultDir"].as<string>();
	}
	if (vm.count("tmpDir"))
	{
		tmpDir = vm["tmpDir"].as<string>();
	}
	if (vm.count("inCountryRaster"))
	{
		inCountryPath = vm["inCountryRaster"].as<string>();
		nParams++;
	}
	if (vm.count("inValueRasterList"))
	{
		inValuePathListRaw = vm["inValueRasterList"].as<string>();
		nParams++;
	}
	if (vm.count("outDir"))
	{
		outDir = vm["outDir"].as<string>();
		nParams++;
	}

	ASSERT_INT(nParams == 3, INCORRECT_INPUT_PARAMS);

	auto inValuePathList = SplitString(inValuePathListRaw, ';');
	raster inCountryRaster(inCountryPath, raster::INPUT);
	for (auto inValuePath : inValuePathList)
	{
		vector<shared_ptr<raster>> inRasterList, outRasterList;
		shared_ptr<raster> inValueRaster(new raster(inValuePath, raster::INPUT));
		shared_ptr<raster> tmpCountrySum(new raster(tmpDir + "countrySum_" + inValueRaster->getName(), raster::TEMPORARY));
		SpatialAnalyst::ZonalStatistics(inCountryRaster, *inValueRaster, *tmpCountrySum, SpatialAnalyst::SUM);
		shared_ptr<raster> outRaster(new raster(resultDir + "mask_" + inValueRaster->getName(), raster::OUTPUT));
		inRasterList.push_back(inValueRaster);
		inRasterList.push_back(tmpCountrySum);
		outRasterList.push_back(outRaster);
	}

	cout << getTime() << endl;
	return 0;
}