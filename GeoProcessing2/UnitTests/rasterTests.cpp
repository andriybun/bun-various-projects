#include "stdafx.h"
#include "CppUnitTest.h"

#include <string>
#include "raster.h"
#include "SpatialAnalyst.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace UnitTests
{		
	void processMultipleRasters(const vector<float> & vec,
		const vector<float> & noDataInVec,
		const vector<float> & noDataOutVec,
		vector<float> & outVec)
	{
		size_t nEl = vec.size();
		outVec[0] = 0;
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

	TEST_CLASS(UnitTestRaster)
	{
		
	public:
		
		TEST_METHOD(TestGetCellArea)
		{
			string home("D:\\Workspace\\IIASA\\GeoProcessing2\\_testData\\");
			raster inRaster(home + "increment", raster::INPUT);
			Assert::AreEqual((float)1, inRaster.getCellValue(1, 4));
			Assert::AreEqual((float)16, inRaster.getCellValue(4, 1));
		}

		TEST_METHOD(TestRasterArithmetics)
		{
			string home("D:\\Workspace\\IIASA\\GeoProcessing2\\_testData\\");
			raster rasIncrement(home + "increment", raster::INPUT);
			raster rasOnes(home + "ones", raster::INPUT);
			raster rasOut(home + "x_unit_test_result", raster::TEMPORARY);
			SpatialAnalyst::RasterArithmetics(&xplus, rasIncrement, rasOnes, rasOut);
			Assert::AreEqual((float)2, rasOut.getCellValue(1, 4));
			Assert::AreEqual((float)3, rasOut.getCellValue(2, 4));
			Assert::AreEqual((float)4, rasOut.getCellValue(3, 4));
			Assert::AreEqual((float)5, rasOut.getCellValue(4, 4));
			Assert::AreEqual((float)14, rasOut.getCellValue(1, 1));
			Assert::AreEqual((float)15, rasOut.getCellValue(2, 1));
			Assert::AreEqual((float)16, rasOut.getCellValue(3, 1));
			Assert::AreEqual((float)17, rasOut.getCellValue(4, 1));
		}

		TEST_METHOD(TestMultipleRasterArithmetics)
		{
			string home("D:\\Workspace\\IIASA\\GeoProcessing2\\_testData\\");
			raster rasIncrement(home + "increment", raster::INPUT);
			raster rasOnes(home + "ones", raster::INPUT);
			raster rasOut(home + "x_unit_test_result", raster::TEMPORARY);

			vector<raster *> inRasterVec, outRasterVec;
			inRasterVec.push_back(&rasIncrement);
			inRasterVec.push_back(&rasOnes);
			outRasterVec.push_back(&rasOut);

			SpatialAnalyst::MultipleRasterArithmetics(&processMultipleRasters, 
				inRasterVec, outRasterVec);

			Assert::AreEqual((float)2, rasOut.getCellValue(1, 4));
			Assert::AreEqual((float)3, rasOut.getCellValue(2, 4));
			Assert::AreEqual((float)4, rasOut.getCellValue(3, 4));
			Assert::AreEqual((float)5, rasOut.getCellValue(4, 4));
			Assert::AreEqual((float)14, rasOut.getCellValue(1, 1));
			Assert::AreEqual((float)15, rasOut.getCellValue(2, 1));
			Assert::AreEqual((float)16, rasOut.getCellValue(3, 1));
			Assert::AreEqual((float)17, rasOut.getCellValue(4, 1));
		}

		TEST_METHOD(TestZonalStatisticsAsTable)
		{
			string home("D:\\Workspace\\IIASA\\GeoProcessing2\\_testData\\");
			raster rasZones(home + "zones", raster::INPUT);
			raster rasValues(home + "increment", raster::INPUT);

			SpatialAnalyst::zonalStatisticsTableT statTable;
			SpatialAnalyst::ZonalStatisticsAsTable(rasZones, 
				rasValues, 
				statTable);

			Assert::AreEqual((float)10, statTable[1].sumVal);
			Assert::AreEqual((float)6.5, statTable[2].meanVal);
			Assert::AreEqual((float)12, statTable[3].maxVal);
			Assert::AreEqual(4, statTable[4].count);
		}

		TEST_METHOD(TestZonalStatistics)
		{
			string home("D:\\Workspace\\IIASA\\GeoProcessing2\\_testData\\");
			raster rasZones(home + "zones", raster::INPUT);
			raster rasValues(home + "increment", raster::INPUT);
			raster rasOut(home + "x_zonal_stat", raster::TEMPORARY);
			
			SpatialAnalyst::ZonalStatistics(rasZones, rasValues, rasOut, SpatialAnalyst::SUM);

			Assert::AreEqual((float)10, rasOut.getCellValue(1, 4));
			Assert::AreEqual((float)26, rasOut.getCellValue(1, 3));
			Assert::AreEqual((float)42, rasOut.getCellValue(1, 2));
			Assert::AreEqual((float)58, rasOut.getCellValue(1, 1));
		}
	};
}