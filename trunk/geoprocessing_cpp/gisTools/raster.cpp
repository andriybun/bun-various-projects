#include "raster.h"

raster::raster(const string & rasterName)
{
	rasterPath = rasterName;

	ifstream f;
	string fileName = rasterName + ".hdr";

	f.open(fileName.c_str(), ios::in);
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
		cout << "Initialized empty raster" << endl;
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

bool raster::validateExtent(const raster & other)
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

void raster::copyFile(const string & source, const string & destination)
{
#if defined(_WIN32) || defined(_WIN64) || defined(OS_WINDOWS)
	system((string("copy \"") + source.c_str() + "\" \"" + destination.c_str() + "\"").c_str());
#else
	system((string("cp \"") + thisHdrPath.c_str() + "\" \"" + outHdrPath.c_str() + "\"").c_str());
#endif
}


// TODO: replace with one map<float, structureWithStatistics> - should be more efficient
//void raster::incMap(map<float, float> &mp, float key, float val)
//{
//	val += mp[key];
//	mp.erase(key);
//	mp.insert(make_pair<float, float>(key, val));
//}
//
//void raster::incMap(map<float, int> &mp, float key, float val)
//{
//	val += mp[key];
//	mp.erase(key);
//	mp.insert(make_pair<float, int>(key, val));
//}

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

void raster::copy(const raster & destinationRaster)
{
	string destinationName = destinationRaster.rasterPath;
	copyFile(rasterPath + ".hdr", destinationName + ".hdr");
	copyFile(rasterPath + ".flt", destinationName + ".flt");
}

void raster::removeFromDisc()
{
	DeleteFile((LPCTSTR)((rasterPath + ".hdr").c_str()));
	DeleteFile((LPCTSTR)((rasterPath + ".flt").c_str()));
}

void raster::rasterArithmetics(float (*func)(float, float), const float num, const raster & outRaster)
{
	string thisHdrPath = (*this).rasterPath + ".hdr";
	string thisFltPath = (*this).rasterPath + ".flt";
	string outHdrPath = outRaster.rasterPath + ".hdr";
	string outFltPath = outRaster.rasterPath + ".flt";

	copyFile(thisHdrPath, outHdrPath);

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

void raster::rasterArithmetics(float (*func)(float, float), const raster & inRaster, const raster & outRaster)
{
	validateExtent(inRaster);

	string thisHdrPath = (*this).rasterPath + ".hdr";
	string thisFltPath = (*this).rasterPath + ".flt";
	string inHdrPath = inRaster.rasterPath + ".hdr";
	string inFltPath = inRaster.rasterPath + ".flt";
	string outHdrPath = outRaster.rasterPath + ".hdr";
	string outFltPath = outRaster.rasterPath + ".flt";

	copyFile(thisHdrPath, outHdrPath);

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

void raster::zonalStatistics(const raster & inZoneRaster, const raster & outRaster, statisticsTypeT statisticsType)
{
	validateExtent(inZoneRaster);

	string thisHdrPath = (*this).rasterPath + ".hdr";
	string thisFltPath = (*this).rasterPath + ".flt";
	string inZoneHdrPath = inZoneRaster.rasterPath + ".hdr";
	string inZoneFltPath = inZoneRaster.rasterPath + ".flt";
	string outHdrPath = outRaster.rasterPath + ".hdr";
	string outFltPath = outRaster.rasterPath + ".flt";

	copyFile(thisHdrPath, outHdrPath);

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