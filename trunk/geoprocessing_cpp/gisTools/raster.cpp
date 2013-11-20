//#include <Python.h>
#include "raster.h"

raster::raster()
{
	isDescribed = false;
	initializedFromImg = false;
	rasterType = EMPTY;
}

raster::raster(const string & rasterFullPath, rasterTypeT rType)
{
	rasterInitPrivate(rasterFullPath, rType);
}

void raster::rasterInit(const string & rasterFullPath, rasterTypeT rType)
{
	// This method can be only called if current raster is EMPTY
	ASSERT_INT(rasterType == EMPTY, ATTEMPT_TO_REINITIALIZE_RASTER)
	rasterInitPrivate(rasterFullPath, rType);
}

void raster::rasterInitPrivate(const string & rasterFullPath, rasterTypeT rType)
{
	if (!strcmp(rasterFullPath.c_str(), "#"))
	{
		printf("# - empty raster\n");
		rasterType = EMPTY;
		return;
	}
	rasterPath = rasterFullPath;
	char fileName[200];
	_splitpath(rasterPath.c_str(), NULL, NULL, fileName, NULL);
	rasterName = fileName;
	isDescribed = false;
	initializedFromImg = false;
	rasterType = rType;
	printf("Raster name: %s\n", rasterPath.c_str());
	if ((rasterType == INPUT) || (rasterType == PASS_INPUT))
	{
		if (!readRasterProperties())
		{
			ifstream f;
			string imgFileName = rasterFullPath + ".img";
			f.open(imgFileName.c_str(), ios::in);
			if (f.is_open())
			{
				f.close();
				convertRasterToFloat();
				readRasterProperties();
				initializedFromImg = true;
				printf("\tInitialized float raster from img-file\n");
			}
			else
			{
				printf("Raster does not exist!\n");
				ASSERT_INT(false, INPUT_RASTER_DOES_NOT_EXIST);
			}
		}
	}
	else
	{
		switch (rasterType)
		{
		case OUTPUT:
			// Nothing is needed to be created at this stage
			printf("\tInitialized output raster\n");
			break;
		case TEMPORARY:
			// Nothing is needed to be created at this stage
			printf("\tInitialized temporary raster\n");
			break;
		case PASS_TEMPORARY:
			// Nothing is needed to be created at this stage
			printf("\tInitialized temporary raster\n");
			break;
		case COPY:
			// Copy can not be created using constructor
			ASSERT_INT(false, OTHER_ERROR);
			break;
		};
	}
}

// Copy constructor
raster::raster(const raster & g)
{
	initializedFromImg = false;
	rasterType = COPY;
	rasterName = g.rasterName;
	rasterPath = g.rasterPath;
	g.copyProperties(*this);
}

// Assignment operator overloading
raster & raster::operator = (const raster & g)
{
	initializedFromImg = false;
	if (this != &g)
	{
		rasterPath = g.rasterPath;
		rasterName = g.rasterName;
		switch (g.rasterType)
		{
		case EMPTY:
			rasterType = g.rasterType;
			break;
		case PASS_INPUT:
			rasterType = INPUT;
			break;
		case PASS_TEMPORARY:
			rasterType = TEMPORARY;
			break;
		default:
			rasterType = COPY;
			break;
		};
		g.copyProperties(*this);
	}
	return *this;
}

// Destructor
raster::~raster()
{
	if (rasterType == OUTPUT)
	{
		convertFloatToRaster();
	}
	if ((initializedFromImg && (rasterType != PASS_INPUT) && (rasterType != PASS_TEMPORARY)) 
		|| (rasterType == TEMPORARY)
		|| (rasterType == INPUT)
		|| (rasterType == OUTPUT))
	{
		removeFloatFromDisc();
	}
}

bool raster::isEmpty()
{
	return (rasterType == EMPTY);
}

bool raster::readRasterProperties()
{
	ifstream f;
	string hdrFileName = rasterPath + ".hdr";
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
		return true;
	}
	else
	{
		return false;
	}
}

bool raster::validateExtent(const raster & other) const
{
	bool result = compare_eq((*this).cellSize, other.cellSize, EPSILON) 
		&& ((*this).horResolution == other.horResolution)
		&& ((*this).verResolution == other.verResolution)
		&& compare_eq((*this).xMin, other.xMin, EPSILON)
		&& compare_eq((*this).yMin, other.yMin, EPSILON);
	if (!result)
	{
		printf("Error: input rasters' extents and/or cellsize disagree!\n");
		if (!compare_eq((*this).cellSize, other.cellSize, EPSILON))
		{
			printf("cell size:      %f - %f\n", (*this).cellSize, other.cellSize);
		}
		if (horResolution != other.horResolution)
		{
			printf("hor resolution: %d - %d\n", (*this).horResolution, other.horResolution);
		}
		if (verResolution != other.verResolution)
		{
			printf("ver resolution: %d - %d\n", (*this).verResolution, other.verResolution);
		}
		if (!compare_eq((*this).xMin, other.xMin, EPSILON))
		{
			printf("x min:          %f - %f\n", (*this).xMin, other.xMin);
		}
		if (!compare_eq((*this).yMin, other.yMin, EPSILON))
		{
			printf("y min:          %f - %f\n", (*this).yMin, other.yMin);
		}
	}
	ASSERT_INT(result, WRONG_EXTENT);
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
	system((string("copy \"") + source + "\" \"" + destination + "\"").c_str());
#else
	system((string("cp \"") + source + "\" \"" + destination + "\"").c_str());
#endif
}

void raster::deleteFile(const string & fileName) const
{
#if defined(_WIN32) || defined(_WIN64) || defined(OS_WINDOWS)
	cout << string("del \"") + fileName + "\"" << endl;
	system((string("del \"") + fileName + "\"").c_str());
#else
	system((string("rm \"") + fileName + "\" \"").c_str());
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
	destination.isDescribed = isDescribed;
	destination.description = description;
}

void raster::incMap(zonalStatisticsTableT &mp, int key, float val)
{
	statisticsStructT tmp = mp[key];
	if (tmp.count == 0)
	{
		tmp.sumVal = val;
		tmp.maxVal = val;
		tmp.minVal = val;
		tmp.count = 1;
		tmp.countNonZero = 1;
	}
	else
	{
		tmp.sumVal += val;
		tmp.maxVal = xmax(tmp.maxVal, val);
		tmp.minVal = xmin(tmp.minVal, val);
		tmp.count++;
		tmp.countNonZero += (val > 0) ? 1 : 0;
	}
	mp.erase(key);
	mp.insert(make_pair<int, statisticsStructT>(key, tmp));
}

raster::statisticsStructT raster::describe()
{
	if (isDescribed)
	{
		return description;
	}

	printf("Computing statistics\n");

	statisticsStructT result;
	result.count = 0;
	result.maxVal = 0;
	result.minVal = 0;
	result.meanVal = 0;
	result.sumVal = 0;

	string fltPath = rasterPath + ".flt";
	ifstream thisFile;
	thisFile.open(fltPath.c_str(), ios::out | ios::binary);
	ASSERT_INT(thisFile.is_open(), FILE_NOT_OPEN);

	int numCells = (*this).horResolution * (*this).verResolution;
	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);
	float * buf = new float[bufSize];

	int numCellsProcessed = 0;
	bool minMaxInitialized = false;
	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		thisFile.read(reinterpret_cast<char*>(buf), sizeof(float) * bufSize);
		for (int i = 0; i < bufSize; i++)
		{
			if (buf[i] != (*this).noDataValue)
			{
				if (!minMaxInitialized)
				{
					result.maxVal = buf[i];
					result.minVal = buf[i];
					minMaxInitialized = true;
				}
				result.count++;
				result.sumVal += buf[i];
				result.maxVal = xmax(buf[i], result.maxVal);
				result.minVal = xmin(buf[i], result.minVal);
			}
		}
		numCellsProcessed += bufSize;
	}

	result.meanVal = result.sumVal / result.count;

	delete [] buf;
	thisFile.close();

	isDescribed = true;
	description = result;

	return result;
}

// Geoprocessing methods:
raster raster::copy(const string & destinationName)
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

void raster::removeFloatFromDisc()
{
	deleteFile((rasterPath + ".hdr").c_str());
	deleteFile((rasterPath + ".flt").c_str());
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
	ASSERT_INT(thisFile.is_open(), FILE_NOT_OPEN);
	ifstream inFile;
	inFile.open(inFltPath.c_str(), ios::out | ios::binary);
	ASSERT_INT(inFile.is_open(), FILE_NOT_OPEN);
	ofstream outFile;
	outFile.open(outFltPath.c_str(), ios::out | ios::binary);
	ASSERT_INT(outFile.is_open(), FILE_NOT_OPEN);

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
			if ((buf[i] != (*this).noDataValue) && (buf2[i] != inRaster.noDataValue))
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

void raster::zonalStatisticsAsTable(const raster & inZoneRaster,
									zonalStatisticsTableT & zonalStatisticsTable,
									statisticsTypeT statisticsType)
{
	printf("Executing zonal statistics (as table)\n");
	validateExtent(inZoneRaster);
	ASSERT_INT(zonalStatisticsTable.size() == 0, OTHER_ERROR);

	string thisHdrPath = (*this).rasterPath + ".hdr";
	string thisFltPath = (*this).rasterPath + ".flt";
	string inZoneHdrPath = inZoneRaster.rasterPath + ".hdr";
	string inZoneFltPath = inZoneRaster.rasterPath + ".flt";

	ifstream thisFile;
	thisFile.open(thisFltPath.c_str(), ios::in | ios::binary);
	ifstream inZoneFile;
	inZoneFile.open(inZoneFltPath.c_str(), ios::out | ios::binary);

	int numCells = (*this).horResolution * (*this).verResolution;
	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);

	float * buf = new float[bufSize];
	float * bufZone = new float[bufSize];

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
			if ((buf[i] != (*this).noDataValue) && (bufZone[i] != inZoneRaster.noDataValue))
			{
				incMap(zonalStatisticsTable, (int)bufZone[i], buf[i]);
			}
		}
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}
	thisFile.close();
	inZoneFile.close();

	delete [] buf;
	delete [] bufZone;
}

void raster::zonalStatistics(const raster & inZoneRaster,
							 raster & outRaster,
							 statisticsTypeT statisticsType)
{
	printf("Executing zonal statistics\n");
	validateExtent(inZoneRaster);

	string inZoneHdrPath = inZoneRaster.rasterPath + ".hdr";
	string inZoneFltPath = inZoneRaster.rasterPath + ".flt";
	string outHdrPath = outRaster.rasterPath + ".hdr";
	string outFltPath = outRaster.rasterPath + ".flt";

	copyFile(inZoneHdrPath, outHdrPath);
	copyProperties(outRaster);

	// First run: compute statistics
	zonalStatisticsTableT zonalStatisticsTable;
	zonalStatisticsAsTable(inZoneRaster, zonalStatisticsTable, statisticsType);

	ifstream inZoneFile;
	inZoneFile.open(inZoneFltPath.c_str(), ios::out | ios::binary);
	ofstream outFile;
	outFile.open(outFltPath.c_str(), ios::out | ios::binary);

	int numCells = (*this).horResolution * (*this).verResolution;
	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);

	float * outBuf = new float[bufSize];
	float * bufZone = new float[bufSize];
	
	// Second run: write statistics to out file
	printf("Second run: writing statistics to file\n");
	
	int numCellsProcessed = 0;
	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		numCellsProcessed += bufSize;
		inZoneFile.read(reinterpret_cast<char*>(bufZone), sizeof(float) * bufSize);
		for (int i = 0; i < bufSize; i++)
		{
			if (bufZone[i] != inZoneRaster.noDataValue)
			{
				statisticsStructT tmp = zonalStatisticsTable[(int)bufZone[i]];
				switch (statisticsType)
				{
				case SUM:
					outBuf[i] = tmp.sumVal;
					break;
				case MEAN:
					outBuf[i] = tmp.sumVal / (float)tmp.count;
					break;
				case MAX:
					outBuf[i] = tmp.maxVal;
					break;
				case MIN:
					outBuf[i] = tmp.minVal;
					break;
				case COUNT:
					outBuf[i] = (float)tmp.count;
					break;
				case COUNT_NON_ZERO:
					outBuf[i] = (float)tmp.countNonZero;
					break;
				}
			}
			else
			{
				outBuf[i] = (*this).noDataValue;
			}
		}
		outFile.write(reinterpret_cast<char *>(outBuf), sizeof(float) * bufSize);
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}
	inZoneFile.close();
	outFile.close();

	delete [] outBuf;
	delete [] bufZone;
}

void raster::combineAsTable(const vector<raster *> & inRastersVector, tableT & outTable)
{
	ASSERT_INT(!outTable.sized, OTHER_ERROR);
	printf("Executing combine (as table)\n");
	vector<raster *> tmpRastersVector = inRastersVector;
	tmpRastersVector.insert(tmpRastersVector.begin(), this);
	size_t numRasters = tmpRastersVector.size();
	outTable.setNumCols(numRasters);

	vector<string> hdrPathsVector;
	vector<string> fltPathsVector;
	vector<ifstream *> inRasterFileVector;

	hdrPathsVector.resize(numRasters);
	fltPathsVector.resize(numRasters);
	inRasterFileVector.resize(numRasters);

	vector<float *> bufVector;
	bufVector.resize(numRasters);

	hdrPathsVector[0] = tmpRastersVector[0]->rasterPath + ".hdr";
	fltPathsVector[0] = tmpRastersVector[0]->rasterPath + ".flt";

	inRasterFileVector[0] = new ifstream;
	inRasterFileVector[0]->open(fltPathsVector[0].c_str(), ios::in | ios::binary);
	ASSERT_INT(inRasterFileVector[0]->is_open(), FILE_NOT_OPEN);

	int numCells = tmpRastersVector[0]->horResolution * tmpRastersVector[0]->verResolution;
	int bufSize = xmin(numCells, (int)ceil(2. * MAX_READ_BUFFER_ELEMENTS / numRasters));

	bufVector[0] = new float[bufSize];
	float * outBuf = new float[bufSize];

	// Initializing all vectors, buffers etc.
	for (size_t idx = 1; idx < numRasters; idx++)
	{
		tmpRastersVector[0]->validateExtent(*tmpRastersVector[idx]);
		hdrPathsVector[idx] = tmpRastersVector[idx]->rasterPath + ".hdr";
		fltPathsVector[idx] = tmpRastersVector[idx]->rasterPath + ".flt";
		inRasterFileVector[idx] = new ifstream;
		inRasterFileVector[idx]->open(fltPathsVector[idx].c_str(), ios::in | ios::binary);
		ASSERT_INT(inRasterFileVector[idx]->is_open(), FILE_NOT_OPEN);
		bufVector[idx] = new float[bufSize];
	}

	// Main loop
	int numCellsProcessed = 0;
	int tableRowIdx = 0;
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
				if (bufVector[rasterIdx][i] == tmpRastersVector[rasterIdx]->noDataValue) isData = false;
				passArg.push_back(bufVector[rasterIdx][i]);
			}
			if (isData && !outTable.exists(passArg))
			{
				outTable.insert(tableRowIdx++, passArg);
			}
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
	delete [] outBuf;
}

// Conversion methods:
void raster::convertRasterToFloat()
{
	printf("Converting \"%s.img\" to float\n", rasterPath.c_str());
	string conversionCommand = string("convertRasterToFloat.py \"") + rasterPath + ".img\" \"" + rasterPath + ".flt\"\n";
	ASSERT_INT(!system(conversionCommand.c_str()), RASTER_TO_FLOAT_CONVERSION_ERROR);
	//Py_Initialize();
	//string conversionCommand = string("gp.RasterToFloat_conversion(r\"") + rasterPath + ".img\", r\"" + rasterPath + ".flt\")\n";
	//PyRun_SimpleString("import arcgisscripting\n");
	//PyRun_SimpleString("gp = arcgisscripting.create()");
	//PyRun_SimpleString("gp.OverWriteOutput = 1");
	//PyRun_SimpleString(conversionCommand.c_str());
	//Py_Finalize();
}

void raster::convertFloatToRaster()
{
	printf("Converting \"%s.flt\" to raster\n", rasterPath.c_str());
	string conversionCommand = string("convertFloatToRaster.py \"") + rasterPath + ".flt\" \"" + rasterPath + ".img\"\n";
	ASSERT_INT(!system(conversionCommand.c_str()), FLOAT_TO_RASTER_CONVERSION_ERROR);
	//Py_Initialize();
	//string conversionCommand = string("gp.FloatToRaster_conversion(r\"") + rasterPath + ".flt\", r\"" + rasterPath + ".img\")\n";
	//PyRun_SimpleString("import arcgisscripting\n");
	//PyRun_SimpleString("gp = arcgisscripting.create()");
	//PyRun_SimpleString("gp.OverWriteOutput = 1");
	//PyRun_SimpleString(conversionCommand.c_str());
	//Py_Finalize();
}

string raster::getHdrPath()
{
	return rasterPath + ".hdr";
}

string raster::getFltPath()
{
	return rasterPath + ".flt";
}

void raster::zonalSumByClassAsTable(const raster & inZoneRaster,
									raster & inClassRaster,
									summaryTableT & calibratedResults,
									const runParamsT & runParams)
{
	printf("Executing zonal sum by class (as table)\n");

	validateExtent(inZoneRaster);
	validateExtent(inClassRaster);
	ASSERT_INT(calibratedResults.size() == 0, OTHER_ERROR);

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

	// If minimum class is less than zero, the probability class file is corrupt
	ASSERT_INT(minClass >= 0, WRONG_PROPERTIES_OF_RASTER);
	// If zero probability class exists, it is ignored
	minClass = xmax(minClass, 1);

	ifstream thisFile;
	thisFile.open(thisFltPath.c_str(), ios::in | ios::binary);
	ASSERT_INT(thisFile.is_open(), FILE_NOT_OPEN);
	ifstream inZoneFile;
	inZoneFile.open(inZoneFltPath.c_str(), ios::out | ios::binary);
	ASSERT_INT(inZoneFile.is_open(), FILE_NOT_OPEN);
	ifstream inClassFile;
	inClassFile.open(inClassFltPath.c_str(), ios::out | ios::binary);
	ASSERT_INT(inClassFile.is_open(), FILE_NOT_OPEN);

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
			if ((bufArea[i] != (*this).noDataValue) &&				// skip cells with at least one noData value
				(bufZone[i] != inZoneRaster.noDataValue) &&
				(bufClass[i] != inClassRaster.noDataValue) &&
				(bufClass[i] != 0))									// and where probability class is zero = no probability
			{
				outTable.inc((int)bufZone[i], (size_t)bufClass[i], bufArea[i]);
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

	string csvTablePath = runParams.resultDir + inZoneRaster.rasterName + "_classes_per_zone.csv";
	FILE * csvTableFile;
	csvTableFile = fopen(csvTablePath.c_str(), "w");
	fprintf(csvTableFile, "Zone ID,Zone stats,Best estimate,Best class,Best class multiplier\n");
	while (row != outTable.data.end())
	{
		int targetSum = row->first;
		float rowSum = (float)0;
		float resultingSum = (float)0;
		float absDiff = (float)targetSum;
		unitResultT rowResult;
		rowResult.bestClass = -1;
		rowResult.bestEstimate = rowSum;
		rowResult.bestClassMultiplier = 1;
		if (targetSum == 0)
		{
			rowResult.error = (float)0;
			rowResult.bestEstimate = (float)0;
			rowResult.bestClass = maxClass + 1;
		}
		else
		{
			for (int cl = (int)maxClass; cl >= (int)minClass; cl--)
			{
				float nextRowSum = rowSum + row->second[cl-1];
				rowResult.bestEstimate = rowSum;
				rowResult.bestClass = cl;
				if (nextRowSum > (float)targetSum)
				{
					rowResult.bestClassMultiplier = ((float)targetSum - rowSum) / row->second[cl-1];
					break;
				}
				rowSum = nextRowSum;
			}
			rowResult.error = (float)0;
		}
		fprintf(csvTableFile, "%d,%d,%f,%d,%f\n", currentCountry, targetSum, rowResult.bestEstimate, rowResult.bestClass, rowResult.bestClassMultiplier);
		calibratedResults.insert(make_pair<int, unitResultT>(row->first, rowResult));
		currentCountry++;
		row++;
	}
	fclose(csvTableFile);
}

float xplus(float val1, float val2)
{
	return val1 + val2;
}

float xminus(float val1, float val2)
{
	return val1 - val2;
}

float xtimes(float val1, float val2)
{
	return val1 * val2;
}

float xdivide(float val1, float val2)
{
	return val1 / val2;
}