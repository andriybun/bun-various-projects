#include "stdafx.h"
#include "CppUnitTest.h"

#include "raster.h"
#include "SpatialAnalyst.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{		
	TEST_CLASS(UnitTestRaster)
	{
	public:
		
		TEST_METHOD(TestGetCellArea)
		{
			raster inRaster("incrementHdr", raster::INPUT);
			Assert::AreEqual(inRaster.getCellValue(1, 1), (float)1);
		}

	};
}