#include <Python.h>
#include "raster.h"

void raster::zonalSumByClassAsTable(const raster & inZoneRaster, raster & inClassRaster, tableT & outTable)
{
	validateExtent(inZoneRaster);
	validateExtent(inClassRaster);
	ASSERT_INT(outTable.size() == 0);

	printf("Executing zonal statistics\n");

	string thisHdrPath = (*this).rasterPath + ".hdr";
	string thisFltPath = (*this).rasterPath + ".flt";
	string inZoneHdrPath = inZoneRaster.rasterPath + ".hdr";
	string inZoneFltPath = inZoneRaster.rasterPath + ".flt";
	string inClassHdrPath = inClassRaster.rasterPath + ".hdr";
	string inClassFltPath = inClassRaster.rasterPath + ".flt";

	// First run: computing statistics for probability raster
	statisticsStructT probabilityStatistics = inClassRaster.describe();
	size_t maxClass = (size_t)probabilityStatistics.maxVal;
	size_t minClass = (size_t)probabilityStatistics.minVal;
	outTable.setNumCols(maxClass);
	printf("\tmin class: %d\n", minClass);
	printf("\tmax class: %d\n", maxClass);

	ifstream thisFile;
	thisFile.open(thisFltPath.c_str(), ios::in | ios::binary);
	ASSERT_INT(thisFile.is_open());
	ifstream inZoneFile;
	inZoneFile.open(inZoneFltPath.c_str(), ios::out | ios::binary);
	ASSERT_INT(inZoneFile.is_open());
	ifstream inClassFile;
	inClassFile.open(inClassFltPath.c_str(), ios::out | ios::binary);
	ASSERT_INT(inClassFile.is_open());

	int numCells = (*this).horResolution * (*this).verResolution;
	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);

	float * bufArea = new float[bufSize];
	float * bufZone = new float[bufSize];
	float * bufClass = new float[bufSize];

	int numCellsProcessed = 0;

	// Second run: compute statistics for classes per country
	printf("Computing statistics for classes per country\n");
	numCellsProcessed = 0;
	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		numCellsProcessed += bufSize;
		thisFile.read(reinterpret_cast<char*>(bufArea), sizeof(float) * bufSize);
		inZoneFile.read(reinterpret_cast<char*>(bufZone), sizeof(float) * bufSize);
		inClassFile.read(reinterpret_cast<char*>(bufClass), sizeof(float) * bufSize);
		for (int i = 0; i < bufSize; i++)
		{
			if ((bufArea[i] != (*this).noDataValue) &&
				(bufZone[i] != inZoneRaster.noDataValue) &&
				(bufClass[i] != inClassRaster.noDataValue))
			{
				outTable.inc(bufZone[i], (size_t)bufClass[i], bufArea[i]);
			}
		}
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}

	thisFile.close();
	inZoneFile.close();
	inClassFile.close();

	delete [] bufArea;
	delete [] bufZone;
	delete [] bufClass;

	// Processing table with computed on previous step statistics
	printf("Analyzing collected statistics\n");

	struct unitResultT
	{
		int bestClass;
		float bestEstimate;
		float error;

	};
	map <float, unitResultT> calibratedResults;

	tableT::dataT::iterator row = outTable.data.begin();
	size_t currentCountry = 0;
	while (row != outTable.data.end())
	{
		float targetSum = row->first;
		cout << "target sum = " << targetSum << endl;
		float rowSum = (float)0;
		float resultingSum = (float)0;
		float absDiff = targetSum;
		unitResultT rowResult;
		rowResult.bestClass = -1;
		rowResult.bestEstimate = rowSum;
		cout << maxClass-1 << "\t" << minClass-1 << endl;
		for (int cl = (int)maxClass-1; cl >= (int)minClass-1; cl--)
		{
			cout << "\t->" << cl << "\t" << minClass-1 << "\n";
			cout << "\t" << cl << "\t" << row->second[cl] << endl;
			rowSum += row->second[cl];
			cout << "\trowSum\n";
			float curDiff = fabs(targetSum - rowSum);
			cout << "\tcurDiff\n";
			if ((curDiff <= absDiff) || (rowResult.bestEstimate == (float)0))
			{
				cout << "\t++\n";
				absDiff = curDiff;
				rowResult.bestEstimate = rowSum;
				rowResult.bestClass = cl;
			}
		}
		rowResult.error = absDiff;
		printf("%d\t%f\t%f\t%d\t%f\n", currentCountry, targetSum, rowResult.bestEstimate, rowResult.bestClass, rowResult.error);
		calibratedResults.insert(make_pair<float, unitResultT>(row->first, rowResult));
		system("pause");
		currentCountry++;
		row++;
	}
}