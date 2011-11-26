#include <Python.h>
#include "raster.h"

raster::raster(const string & rasterName)
{
	rasterPath = rasterName;

	ifstream f;
	string hdrFileName = rasterName + ".hdr";
	string imgFileName = rasterName + ".img";

	f.open(hdrFileName.c_str(), ios::in);
	if (f.is_open())
	{
		string line;
		{
			getline(f, line);
			stringstream ss(line);
			ss >> line;
			ss >> horResolution;
		}
		{
			getline(f, line);
			stringstream ss(line);
			ss >> line;
			ss >> verResolution;
		}
		{
			getline(f, line);
			replace(line.begin(), line.end(), ',', '.');
			stringstream ss(line);
			ss >> line;
			ss >> xMin;
		}
		{
			getline(f, line);
			replace(line.begin(), line.end(), ',', '.');
			stringstream ss(line);
			ss >> line;
			ss >> yMin;
		}
		{
			getline(f, line);
			replace(line.begin(), line.end(), ',', '.');
			stringstream ss(line);
			ss >> line;
			ss >> cellSize;
		}
		{
			getline(f, line);
			replace(line.begin(), line.end(), ',', '.');
			stringstream ss(line);
			ss >> line;
			ss >> noDataValue;
		}
		f.close();
		cout << "Initialized raster from: " << rasterName << endl;
	}
	else
	{
		f.open(imgFileName.c_str(), ios::in);
		if (f.is_open())
		{
			f.close();
			convertRasterToFloat();
		}
		else
		{
			cout << "Initialized empty raster" << endl;
		}
	}
}

// Copy constructor
raster::raster(const raster & g)
{
	rasterPath = g.rasterPath;
	horResolution = g.horResolution;
	verResolution = g.verResolution;
	xMin = g.xMin;
	yMin = g.yMin;
	cellSize = g.cellSize;
	noDataValue = g.noDataValue;
}

// Assignment operator overloading
raster & raster::operator = (const raster & g)
{
	if (this != &g)
	{
		rasterPath = g.rasterPath;
		horResolution = g.horResolution;
		verResolution = g.verResolution;
		xMin = g.xMin;
		yMin = g.yMin;
		cellSize = g.cellSize;
		noDataValue = g.noDataValue;
	}
	return *this;
}

// Destructor
raster::~raster()
{

}

bool raster::validateExtent(const raster & other) const
{
	bool result = ((*this).cellSize == other.cellSize) 
		&& ((*this).horResolution == other.horResolution)
		&& ((*this).verResolution == other.verResolution)
		&& ((*this).xMin == other.xMin)
		&& ((*this).yMin == other.yMin);
	assert(result);
	return result;
}

void raster::saveHdr()
{
	ofstream file;
	file.open((rasterPath + ".hdr").c_str(), ios::out);
	file << "ncols         " << horResolution << endl;
	file << "nrows         " << verResolution << endl;
	file << "xllcorner     " << xMin << endl;
	file << "yllcorner     " << yMin << endl;
	file << "cellsize      " << cellSize << endl;
	file << "NODATA_value  " << noDataValue << endl;
	file << "byteorder     LSBFIRST" << endl;
	file.close();
}

bool raster::fileExists(const string & fileName)
{
	ifstream ifile(fileName.c_str());
	if (ifile)
	{
		return true;
	}
	return false;
}

void raster::copyFile(const string & source, const string & destination) const
{
#if defined(_WIN32) || defined(_WIN64) || defined(OS_WINDOWS)
	system((string("copy \"") + source.c_str() + "\" \"" + destination.c_str() + "\"").c_str());
#else
	system((string("cp \"") + thisHdrPath.c_str() + "\" \"" + outHdrPath.c_str() + "\"").c_str());
#endif
}

void raster::copyProperties(raster & destination) const
{
	destination.horResolution = horResolution;
	destination.verResolution = verResolution;
	destination.xMin = xMin;
	destination.yMin = yMin;
	destination.cellSize = cellSize;
	destination.noDataValue = noDataValue;
}

void raster::incMap(map<float, statisticsStructT> &mp, float key, float val)
{
	statisticsStructT tmp = mp[key];
	if (tmp.count == 0)
	{
		tmp.sumVal = val;
		tmp.maxVal = val;
		tmp.minVal = val;
		tmp.count = 1;
	}
	else
	{
		tmp.sumVal += val;
		tmp.maxVal = xmax(tmp.maxVal, val);
		tmp.minVal = xmin(tmp.minVal, val);
		tmp.count++;
	}
	mp.erase(key);
	mp.insert(make_pair<float, statisticsStructT>(key, tmp));
}

// Geoprocessing methods:
raster & raster::copy(const string & destinationName)
{
	copyFile(rasterPath + ".hdr", destinationName + ".hdr");
	copyFile(rasterPath + ".flt", destinationName + ".flt");
	raster destinationRaster = (*this);
	destinationRaster.rasterPath = destinationName;
	return destinationRaster;
}

void raster::copy(raster & destinationRaster)
{
	string destinationName = destinationRaster.rasterPath;
	copyFile(rasterPath + ".hdr", destinationName + ".hdr");
	copyFile(rasterPath + ".flt", destinationName + ".flt");
	copyProperties(destinationRaster);
}

void raster::removeFromDisc()
{
	DeleteFile((LPCTSTR)((rasterPath + ".hdr").c_str()));
	DeleteFile((LPCTSTR)((rasterPath + ".flt").c_str()));
}

void raster::rasterArithmetics(float (*func)(float, float), const float num, raster & outRaster)
{
	printf("Executing raster arithmetics\n");
	string thisHdrPath = (*this).rasterPath + ".hdr";
	string thisFltPath = (*this).rasterPath + ".flt";
	string outHdrPath = outRaster.rasterPath + ".hdr";
	string outFltPath = outRaster.rasterPath + ".flt";

	copyFile(thisHdrPath, outHdrPath);
	copyProperties(outRaster);

	ifstream thisFile;
	thisFile.open(thisFltPath.c_str(), ios::in | ios::binary);
	ofstream outFile;
	outFile.open(outFltPath.c_str(), ios::out | ios::binary);

	int numCells = (*this).horResolution * (*this).verResolution;
	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);

	float * buf = new float[bufSize];

	int numCellsProcessed = 0;
	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		numCellsProcessed += bufSize;
		thisFile.read(reinterpret_cast<char*>(buf), sizeof(float) * bufSize);
		for (int i = 0; i < bufSize; i++)
		{
			if (buf[i] != (*this).noDataValue)
			{
				buf[i] = func(buf[i], num);
			}
			else
			{
				buf[i] = (*this).noDataValue;
			}
		}
		outFile.write(reinterpret_cast<char *>(buf), sizeof(float) * bufSize);
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}

	thisFile.close();
	outFile.close();

    delete [] buf;
}

void raster::rasterArithmetics(float (*func)(float, float), const raster & inRaster, raster & outRaster)
{
	printf("Executing raster arithmetics\n");
	validateExtent(inRaster);

	string thisHdrPath = (*this).rasterPath + ".hdr";
	string thisFltPath = (*this).rasterPath + ".flt";
	string inHdrPath = inRaster.rasterPath + ".hdr";
	string inFltPath = inRaster.rasterPath + ".flt";
	string outHdrPath = outRaster.rasterPath + ".hdr";
	string outFltPath = outRaster.rasterPath + ".flt";

	copyFile(thisHdrPath, outHdrPath);
	copyProperties(outRaster);

	ifstream thisFile;
	thisFile.open(thisFltPath.c_str(), ios::in | ios::binary);
	ifstream inFile;
	inFile.open(inFltPath.c_str(), ios::out | ios::binary);
	ofstream outFile;
	outFile.open(outFltPath.c_str(), ios::out | ios::binary);

	int numCells = (*this).horResolution * (*this).verResolution;
	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);

	float * buf = new float[bufSize];
	float * buf2 = new float[bufSize];

	int numCellsProcessed = 0;
	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		numCellsProcessed += bufSize;
		thisFile.read(reinterpret_cast<char*>(buf), sizeof(float) * bufSize);
		inFile.read(reinterpret_cast<char*>(buf2), sizeof(float) * bufSize);
		for (int i = 0; i < bufSize; i++)
		{
			if ((buf[i] != (*this).noDataValue) || (buf2[i] != inRaster.noDataValue))
			{
				buf[i] = func(buf[i], buf2[i]);
			}
			else
			{
				buf[i] = (*this).noDataValue;
			}
		}
		outFile.write(reinterpret_cast<char *>(buf), sizeof(float) * bufSize);
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}

	thisFile.close();
	inFile.close();
	outFile.close();

	delete [] buf;
	delete [] buf2;
}

void raster::zonalStatistics(const raster & inZoneRaster, raster & outRaster, statisticsTypeT statisticsType)
{
	printf("Executing zonal statistics\n");
	validateExtent(inZoneRaster);

	string thisHdrPath = (*this).rasterPath + ".hdr";
	string thisFltPath = (*this).rasterPath + ".flt";
	string inZoneHdrPath = inZoneRaster.rasterPath + ".hdr";
	string inZoneFltPath = inZoneRaster.rasterPath + ".flt";
	string outHdrPath = outRaster.rasterPath + ".hdr";
	string outFltPath = outRaster.rasterPath + ".flt";

	copyFile(thisHdrPath, outHdrPath);
	copyProperties(outRaster);

	ifstream thisFile;
	thisFile.open(thisFltPath.c_str(), ios::in | ios::binary);
	ifstream inZoneFile;
	inZoneFile.open(inZoneFltPath.c_str(), ios::out | ios::binary);
	ofstream outFile;
	outFile.open(outFltPath.c_str(), ios::out | ios::binary);

	int numCells = (*this).horResolution * (*this).verResolution;
	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);

	float * buf = new float[bufSize];
	float * bufZone = new float[bufSize];

	map<float, statisticsStructT> zonalStatTable;

	// First run: compute statistics
	printf("First run: computing statistics\n");
	int numCellsProcessed = 0;
	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		numCellsProcessed += bufSize;
		thisFile.read(reinterpret_cast<char*>(buf), sizeof(float) * bufSize);
		inZoneFile.read(reinterpret_cast<char*>(bufZone), sizeof(float) * bufSize);
		for (int i = 0; i < bufSize; i++)
		{
			if ((buf[i] != (*this).noDataValue) || (bufZone[i] != inZoneRaster.noDataValue))
			{
				incMap(zonalStatTable, bufZone[i], buf[i]);
			}
		}
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}
	// Second run: write statistics to out file
	printf("Second run: writing statistics to file\n");
	bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);
	inZoneFile.seekg(0, ios::beg);
	numCellsProcessed = 0;
	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		numCellsProcessed += bufSize;
		inZoneFile.read(reinterpret_cast<char*>(bufZone), sizeof(float) * bufSize);
		for (int i = 0; i < bufSize; i++)
		{
			if (bufZone[i] != inZoneRaster.noDataValue)
			{
				statisticsStructT tmp = zonalStatTable[bufZone[i]];
				switch (statisticsType)
				{
				case SUM:
					buf[i] = tmp.sumVal;
					break;
				case MEAN:
					buf[i] = tmp.sumVal / tmp.count;
					break;
				case MAX:
					buf[i] = tmp.maxVal;
					break;
				case MIN:
					buf[i] = tmp.minVal;
					break;
				case COUNT:
					buf[i] = (float)tmp.count;
					break;
				}
			}
			else
			{
				buf[i] = (*this).noDataValue;
			}
		}
		outFile.write(reinterpret_cast<char *>(buf), sizeof(float) * bufSize);
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}
	thisFile.close();
	inZoneFile.close();
	outFile.close();

	delete [] buf;
	delete [] bufZone;
}

// Conversion methods:
void raster::convertRasterToFloat()
{
	printf("Converting \"%s.img\" to float\n", rasterPath.c_str());
	Py_Initialize();
	string conversionCommand = string("gp.RasterToFloat_conversion(r\"") + rasterPath + ".img\", r\"" + rasterPath + ".flt\")\n";
	PyRun_SimpleString("import arcgisscripting\n");
	PyRun_SimpleString("gp = arcgisscripting.create()");
	PyRun_SimpleString("gp.OverWriteOutput = 1");
	PyRun_SimpleString(conversionCommand.c_str());
	Py_Finalize();
}

void raster::convertFloatToRaster()
{
	printf("Converting \"%s.flt\" to raster\n", rasterPath.c_str());
	Py_Initialize();
	string conversionCommand = string("gp.FloatToRaster_conversion(r\"") + rasterPath + ".flt\", r\"" + rasterPath + ".img\")\n";
	PyRun_SimpleString("import arcgisscripting\n");
	PyRun_SimpleString("gp = arcgisscripting.create()");
	PyRun_SimpleString("gp.OverWriteOutput = 1");
	PyRun_SimpleString(conversionCommand.c_str());
	Py_Finalize();
}

// Friend functions:
void multipleRasterArithmetics(float (*func)(vector<float>), const vector<raster> & inRastersVector, raster & outRaster)
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

	hdrPathsVector[0] = inRastersVector[0].rasterPath + ".hdr";
	fltPathsVector[0] = inRastersVector[0].rasterPath + ".flt";
	string outHdrPath = outRaster.rasterPath + ".hdr";
	string outFltPath = outRaster.rasterPath + ".flt";

	inRasterFileVector[0] = new ifstream;
	inRasterFileVector[0]->open(fltPathsVector[0].c_str(), ios::in | ios::binary);

	int numCells = inRastersVector[0].horResolution * inRastersVector[0].verResolution;
	int bufSize = xmin(numCells, ceil(2. * MAX_READ_BUFFER_ELEMENTS / numRasters));

	bufVector[0] = new float[bufSize];
	float * outBuf = new float[bufSize];
	
	// Initializing all vectors, buffers etc.
	for (size_t idx = 1; idx < numRasters; idx++)
	{
		inRastersVector[0].validateExtent(inRastersVector[idx]);
		hdrPathsVector[idx] = inRastersVector[idx].rasterPath + ".hdr";
		fltPathsVector[idx] = inRastersVector[idx].rasterPath + ".flt";
		inRasterFileVector[idx] = new ifstream;
		inRasterFileVector[idx]->open(fltPathsVector[idx].c_str(), ios::in | ios::binary);
		bufVector[idx] = new float[bufSize];
	}
	ifstream inr;
	inr.open(fltPathsVector[0].c_str(), ios::in | ios::binary);

	inRastersVector[0].copyFile(hdrPathsVector[0], outHdrPath);
	inRastersVector[0].copyProperties(outRaster);
	outRasterFile.open(outFltPath.c_str(), ios::out | ios::binary);

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
				if (bufVector[rasterIdx][i] == inRastersVector[rasterIdx].noDataValue) isData = false;
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