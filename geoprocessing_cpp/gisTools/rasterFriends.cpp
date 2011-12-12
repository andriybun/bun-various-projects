//#include <Python.h>
#include "raster.h"

// Friend functions:
void multipleRasterArithmetics(float (*func)(const vector<float> &),
							   const vector<raster *> & inRastersVector,
							   raster & outRaster)
{

	printf("Executing multiple raster arithmetics\n");
	size_t numRasters = inRastersVector.size();
	vector<string> hdrPathsVector;
	vector<string> fltPathsVector;
	vector<float *> bufVector;
	vector<ifstream *> inRasterFileVector;
	ofstream outRasterFile;

	hdrPathsVector.resize(numRasters);
	fltPathsVector.resize(numRasters);
	bufVector.resize(numRasters);
	inRasterFileVector.resize(numRasters);

	hdrPathsVector[0] = inRastersVector[0]->rasterPath + ".hdr";
	fltPathsVector[0] = inRastersVector[0]->rasterPath + ".flt";
	string outHdrPath = outRaster.rasterPath + ".hdr";
	string outFltPath = outRaster.rasterPath + ".flt";

	inRasterFileVector[0] = new ifstream;
	inRasterFileVector[0]->open(fltPathsVector[0].c_str(), ios::in | ios::binary);
	ASSERT_INT(inRasterFileVector[0]->is_open(), FILE_NOT_OPEN);

	int numCells = inRastersVector[0]->horResolution * inRastersVector[0]->verResolution;
	int bufSize = xmin(numCells, (int)ceil(2. * MAX_READ_BUFFER_ELEMENTS / numRasters));

	bufVector[0] = new float[bufSize];
	float * outBuf = new float[bufSize];

	// Initializing all vectors, buffers etc.
	for (size_t idx = 1; idx < numRasters; idx++)
	{
		inRastersVector[0]->validateExtent(*inRastersVector[idx]);
		hdrPathsVector[idx] = inRastersVector[idx]->rasterPath + ".hdr";
		fltPathsVector[idx] = inRastersVector[idx]->rasterPath + ".flt";
		inRasterFileVector[idx] = new ifstream;
		inRasterFileVector[idx]->open(fltPathsVector[idx].c_str(), ios::in | ios::binary);
		ASSERT_INT(inRasterFileVector[idx]->is_open(), FILE_NOT_OPEN);
		bufVector[idx] = new float[bufSize];
	}
	inRastersVector[0]->copyFile(hdrPathsVector[0], outHdrPath);
	inRastersVector[0]->copyProperties(outRaster);
	outRasterFile.open(outFltPath.c_str(), ios::out | ios::binary);
	ASSERT_INT(outRasterFile.is_open(), FILE_NOT_OPEN);

	// Main loop
	int numCellsProcessed = 0;
	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		numCellsProcessed += bufSize;
		for (size_t rasterIdx = 0; rasterIdx < numRasters; rasterIdx++)
		{
			inRasterFileVector[rasterIdx]->read(reinterpret_cast<char*>(bufVector[rasterIdx]), sizeof(float) * bufSize);
		}
		for (int i = 0; i < bufSize; i++)
		{
			bool isData = true;
			vector<float> passArg;
			for (size_t rasterIdx = 0; rasterIdx < numRasters; rasterIdx++)
			{
				if (bufVector[rasterIdx][i] == inRastersVector[rasterIdx]->noDataValue) isData = false;
				passArg.push_back(bufVector[rasterIdx][i]);
			}
			if (isData)
			{
				outBuf[i] = func(passArg);
			}
			else
			{
				outBuf[i] = outRaster.noDataValue;
			}
		}
		outRasterFile.write(reinterpret_cast<char *>(outBuf), sizeof(float) * bufSize);
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}

	// Freeing up memory
	for (size_t idx = 0; idx < numRasters; idx++)
	{
		delete [] bufVector[idx];
		inRasterFileVector[idx]->close();
		delete [] inRasterFileVector[idx];
	}
	delete [] outBuf;
	outRasterFile.close();
}

void multipleRasterArithmetics(void (*func)(const vector<float> &, const vector<float> &, const vector<float> &, vector<float> &),
							   const vector<raster *> & inRastersVector,
							   vector<raster *> & outRastersVector)
{
	printf("Executing multiple raster arithmetics\n");
	size_t numRasters = inRastersVector.size();
	size_t numOutRasters = outRastersVector.size();
	vector<string> hdrPathsVector;
	vector<string> fltPathsVector;
	vector<ifstream *> inRasterFileVector;
	vector<ofstream *> outRasterFileVector;
	vector<string> hdrOutPathsVector;
	vector<string> fltOutPathsVector;
	vector<float *> bufVector;
	vector<float *> outBufVector;
	vector<float> noDataValuesVector;
	vector<float> noDataValuesOutVector;

	hdrPathsVector.resize(numRasters);
	fltPathsVector.resize(numRasters);
	bufVector.resize(numRasters);
	outBufVector.resize(numOutRasters);
	inRasterFileVector.resize(numRasters);
	outRasterFileVector.resize(numOutRasters);
	hdrOutPathsVector.resize(numOutRasters);
	fltOutPathsVector.resize(numOutRasters);

	hdrPathsVector[0] = inRastersVector[0]->rasterPath + ".hdr";
	fltPathsVector[0] = inRastersVector[0]->rasterPath + ".flt";

	inRasterFileVector[0] = new ifstream;
	inRasterFileVector[0]->open(fltPathsVector[0].c_str(), ios::in | ios::binary);
	ASSERT_INT(inRasterFileVector[0]->is_open(), FILE_NOT_OPEN);

	int numCells = inRastersVector[0]->horResolution * inRastersVector[0]->verResolution;
	int bufSize = xmin(numCells, (int)ceil(2. * MAX_READ_BUFFER_ELEMENTS / numRasters));

	bufVector[0] = new float[bufSize];

	// Initializing all vectors, buffers etc.
	noDataValuesVector.resize(numRasters);
	noDataValuesVector[0] = inRastersVector[0]->noDataValue;
	for (size_t idx = 1; idx < numRasters; idx++)
	{
		inRastersVector[0]->validateExtent(*inRastersVector[idx]);
		hdrPathsVector[idx] = inRastersVector[idx]->rasterPath + ".hdr";
		fltPathsVector[idx] = inRastersVector[idx]->rasterPath + ".flt";
		inRasterFileVector[idx] = new ifstream;
		inRasterFileVector[idx]->open(fltPathsVector[idx].c_str(), ios::in | ios::binary);
		ASSERT_INT(inRasterFileVector[idx]->is_open(), FILE_NOT_OPEN);
		bufVector[idx] = new float[bufSize];
		noDataValuesVector[idx] = inRastersVector[idx]->noDataValue;
	}
	noDataValuesOutVector.resize(numOutRasters);
	for (size_t idx = 0; idx < numOutRasters; idx++)
	{
		hdrOutPathsVector[idx] = outRastersVector[idx]->rasterPath + ".hdr";
		fltOutPathsVector[idx] = outRastersVector[idx]->rasterPath + ".flt";
		inRastersVector[0]->copyFile(hdrPathsVector[0], hdrOutPathsVector[idx]);
		inRastersVector[0]->copyProperties(*outRastersVector[idx]);
		fltOutPathsVector[idx].c_str();
		outRasterFileVector[idx] = new ofstream;
		outRasterFileVector[idx]->open(fltOutPathsVector[idx].c_str(), ios::out | ios::binary);
		ASSERT_INT(outRasterFileVector[idx]->is_open(), FILE_NOT_OPEN);
		outBufVector[idx] = new float[bufSize];
		noDataValuesOutVector[idx] = outRastersVector[idx]->noDataValue;
	}

	// Main loop
	int numCellsProcessed = 0;
	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		numCellsProcessed += bufSize;
		for (size_t rasterIdx = 0; rasterIdx < numRasters; rasterIdx++)
		{
			inRasterFileVector[rasterIdx]->read(reinterpret_cast<char*>(bufVector[rasterIdx]), sizeof(float) * bufSize);
		}
		for (int i = 0; i < bufSize; i++)
		{
			bool isData = true;
			vector<float> passArg;
			for (size_t rasterIdx = 0; rasterIdx < numRasters; rasterIdx++)
			{
				if (bufVector[rasterIdx][i] == inRastersVector[rasterIdx]->noDataValue) isData = false;
				passArg.push_back(bufVector[rasterIdx][i]);
			}
			vector<float> result;
			result.resize(numOutRasters);
			func(passArg, noDataValuesVector, noDataValuesOutVector, result);
			for (size_t idx = 0; idx < numOutRasters; idx++)
			{
				outBufVector[idx][i] = result[idx];
			}
		}
		for (size_t outFileIdx = 0; outFileIdx < numOutRasters; outFileIdx++)
		{
			outRasterFileVector[outFileIdx]->write(reinterpret_cast<char *>(outBufVector[outFileIdx]), sizeof(float) * bufSize);
		}
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}

	// Freeing up memory
	for (size_t idx = 0; idx < numRasters; idx++)
	{
		delete [] bufVector[idx];
		inRasterFileVector[idx]->close();
		delete [] inRasterFileVector[idx];
	}
	for (size_t idx = 0; idx < numOutRasters; idx++)
	{
		delete [] outBufVector[idx];
		outRasterFileVector[idx]->close();
		delete [] outRasterFileVector[idx];
	}
}

//void multipleRasterArithmeticsAsTable(float (*func)(const vector<float> & , vector<float> &), const vector<raster *> & inRastersVector, tableT & outTable)
//{
//	printf("Executing multiple raster arithmetics as table\n");
//	size_t numRasters = inRastersVector.size();
//	vector<string> hdrPathsVector;
//	vector<string> fltPathsVector;
//	vector<float *> bufVector;
//	vector<ifstream *> inRasterFileVector;
//
//	hdrPathsVector.resize(numRasters);
//	fltPathsVector.resize(numRasters);
//	bufVector.resize(numRasters);
//	inRasterFileVector.resize(numRasters);
//
//	hdrPathsVector[0] = inRastersVector[0].rasterPath + ".hdr";
//	fltPathsVector[0] = inRastersVector[0].rasterPath + ".flt";
//
//	inRasterFileVector[0] = new ifstream;
//	inRasterFileVector[0]->open(fltPathsVector[0].c_str(), ios::in | ios::binary);
//
//	int numCells = inRastersVector[0].horResolution * inRastersVector[0].verResolution;
//	int bufSize = xmin(numCells, ceil(2. * MAX_READ_BUFFER_ELEMENTS / numRasters));
//
//	bufVector[0] = new float[bufSize];
//
//	// Initializing all vectors, buffers etc.
//	for (size_t idx = 1; idx < numRasters; idx++)
//	{
//		inRastersVector[0].validateExtent(inRastersVector[idx]);
//		hdrPathsVector[idx] = inRastersVector[idx].rasterPath + ".hdr";
//		fltPathsVector[idx] = inRastersVector[idx].rasterPath + ".flt";
//		inRasterFileVector[idx] = new ifstream;
//		inRasterFileVector[idx]->open(fltPathsVector[idx].c_str(), ios::in | ios::binary);
//		bufVector[idx] = new float[bufSize];
//	}
//
//	// Main loop
//	int numCellsProcessed = 0;
//	while(numCellsProcessed < numCells)
//	{
//		bufSize = min(bufSize, numCells - numCellsProcessed);
//		numCellsProcessed += bufSize;
//		for (size_t rasterIdx = 0; rasterIdx < numRasters; rasterIdx++)
//		{
//			inRasterFileVector[rasterIdx]->read(reinterpret_cast<char*>(bufVector[rasterIdx]), sizeof(float) * bufSize);
//		}
//		for (int i = 0; i < bufSize; i++)
//		{
//			bool isData = true;
//			vector<float> passArg;
//			for (size_t rasterIdx = 0; rasterIdx < numRasters; rasterIdx++)
//			{
//				if (bufVector[rasterIdx][i] == inRastersVector[rasterIdx].noDataValue) isData = false;
//				passArg.push_back(bufVector[rasterIdx][i]);
//			}
//			if (isData)
//			{
//				vector<float> data;
//				float key = func(passArg, data);
//				outTable.inc(key, data);
//			}
//		}
//		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
//	}
//
//	// Freeing up memory
//	for (size_t idx = 0; idx < numRasters; idx++)
//	{
//		delete [] bufVector[idx];
//		inRasterFileVector[idx]->close();
//		delete [] inRasterFileVector[idx];
//	}
//}
