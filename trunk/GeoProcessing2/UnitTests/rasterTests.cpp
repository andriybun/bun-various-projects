#include "stdafx.h"
#include "CppUnitTest.h"

#include <string>
#include "raster.h"
#include "SpatialAnalyst.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace UnitTests
{		
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

	};
}