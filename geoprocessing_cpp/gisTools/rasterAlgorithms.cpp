#include <Python.h>
#include "raster.h"

void raster::zonalSumByClassAsTable(const raster & inZoneRaster,
									raster & inClassRaster,
									summaryTableT calibratedResults)
{
	validateExtent(inZoneRaster);
	validateExtent(inClassRaster);
	ASSERT_INT(calibratedResults.size() == 0);

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
	tableT outTable;
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

	tableT::dataT::iterator row = outTable.data.begin();
	size_t currentCountry = 0;
	printf("Zone ID\tZone stats\tBest estimate\tBest class\tError\n");
	while (row != outTable.data.end())
	{
		float targetSum = row->first;
		//cout << "target sum = " << targetSum << endl;
		float rowSum = (float)0;
		float resultingSum = (float)0;
		float absDiff = targetSum;
		unitResultT rowResult;
		rowResult.bestClass = -1;
		rowResult.bestEstimate = rowSum;
		//cout << maxClass-1 << "\t" << minClass-1 << endl;
		for (int cl = (int)maxClass; cl >= (int)minClass; cl--)
		{
			//cout << "\t->" << cl << "\t" << minClass-1 << "\n";
			//cout << "\t" << cl << "\t" << row->second[cl] << endl;
			rowSum += row->second[cl-1];
			//cout << "\trowSum\n";
			float curDiff = fabs(targetSum - rowSum);
			//cout << "\tcurDiff\n";
			if ((curDiff <= absDiff) || (rowResult.bestEstimate == (float)0))
			{
				//cout << "\t++\n";
				absDiff = curDiff;
				rowResult.bestEstimate = rowSum;
				rowResult.bestClass = cl;
			}
		}
		rowResult.error = absDiff;
		printf("%d\t%f\t%f\t%d\t\t%f\n", currentCountry, targetSum, rowResult.bestEstimate, rowResult.bestClass, rowResult.error);
		calibratedResults.insert(make_pair<float, unitResultT>(row->first, rowResult));
		currentCountry++;
		row++;
	}
}

void raster::validateCropland(const raster & inZoneRaster,
							  raster & inClassRaster,
							  raster & outCroplandRaster,
							  raster & outErrorRaster)
{
	// Compute statistics
	summaryTableT calibratedResults;
	zonalSumByClassAsTable(inZoneRaster, inClassRaster, calibratedResults);

	// Write statistics to resulting files
	string inCroplandHdrPath = (*this).rasterPath + ".hdr";
	string inCroplandFltPath = (*this).rasterPath + ".flt";
	string inZoneHdrPath = inZoneRaster.rasterPath + ".hdr";
	string inZoneFltPath = inZoneRaster.rasterPath + ".flt";
	string inClassFltPath = inClassRaster.rasterPath + ".flt";
	string inClassHdrPath = inClassRaster.rasterPath + ".hdr";
	string outCroplandHdrPath = outCroplandRaster.rasterPath + ".hdr";
	string outCroplandFltPath = outCroplandRaster.rasterPath + ".flt";
	string outErrorHdrPath = outErrorRaster.rasterPath + ".hdr";
	string outErrorFltPath = outErrorRaster.rasterPath + ".flt";

	copyFile(inZoneHdrPath, outCroplandHdrPath);
	copyFile(inZoneHdrPath, outErrorHdrPath);
	copyProperties(outCroplandRaster);
	copyProperties(outErrorRaster);

	ifstream inCroplandFile;
	inCroplandFile.open(inCroplandFltPath.c_str(), ios::out | ios::binary);
	ifstream inZoneFile;
	inZoneFile.open(inZoneFltPath.c_str(), ios::out | ios::binary);
	ifstream inClassFile;
	inClassFile.open(inClassFltPath.c_str(), ios::out | ios::binary);
	ofstream outCroplandFile;
	outCroplandFile.open(outCroplandFltPath.c_str(), ios::out | ios::binary);
	ofstream outErrorFile;
	outErrorFile.open(outErrorFltPath.c_str(), ios::out | ios::binary);

	int numCells = inZoneRaster.horResolution * inZoneRaster.verResolution;
	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);

	float * inBufCropland = new float[bufSize];
	float * inBufZone = new float[bufSize];
	float * inBufClass = new float[bufSize];
	float * outBufCropland = new float[bufSize];
	float * outBufError = new float[bufSize];

	printf("Writing results to files\n");
	int numCellsProcessed = 0;
	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		numCellsProcessed += bufSize;
		inCroplandFile.read(reinterpret_cast<char*>(inBufCropland), sizeof(float) * bufSize);
		inZoneFile.read(reinterpret_cast<char*>(inBufZone), sizeof(float) * bufSize);
		inClassFile.read(reinterpret_cast<char*>(inBufClass), sizeof(float) * bufSize);
		for (int i = 0; i < bufSize; i++)
		{
			if ((inBufCropland[i] != (*this).noDataValue)
				&& (inBufZone[i] != inZoneRaster.noDataValue)
				&& (inBufClass[i] != inClassRaster.noDataValue))
			{
				unitResultT unitResult = calibratedResults[inBufZone[i]];
				outBufCropland[i] = (unitResult.bestClass <= inBufClass[i]) ? inBufCropland[i] : inZoneRaster.noDataValue;
				outBufError[i] = unitResult.error;
			}
			else
			{
				if (inBufZone[i] != inZoneRaster.noDataValue)
				{
					unitResultT unitResult = calibratedResults[inBufZone[i]];
					outBufError[i] = unitResult.error;
				}
				outBufCropland[i] = inZoneRaster.noDataValue;
			}
		}
		outCroplandFile.write(reinterpret_cast<char *>(outBufCropland), sizeof(float) * bufSize);
		outErrorFile.write(reinterpret_cast<char *>(outBufError), sizeof(float) * bufSize);
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}

	inCroplandFile.close();
	inZoneFile.close();
	inClassFile.close();
	outCroplandFile.close();
	outErrorFile.close();

	delete [] inBufCropland;
	delete [] inBufZone;
	delete [] inBufClass;
	delete [] outBufCropland;
	delete [] outBufError;
}