//#include <Python.h>
#include "raster.h"

raster::raster()
{
	this->isDescribed = false;
	this->initializedFromImg = false;
	this->rasterType = EMPTY;
}

raster::raster(const std::string & rasterFullPath, rasterTypeT rType)
{
	rasterInitPrivate(rasterFullPath, rType);
}

void raster::rasterInit(const std::string & rasterFullPath, rasterTypeT rType)
{
	// This method can be only called if current raster is EMPTY
	ASSERT_INT(rasterType == EMPTY, ATTEMPT_TO_REINITIALIZE_RASTER)
	rasterInitPrivate(rasterFullPath, rType);
}

void raster::rasterInitPrivate(const std::string & rasterFullPath, rasterTypeT rType)
{
	if (!strcmp(rasterFullPath.c_str(), "#"))
	{
		printf("# - empty raster\n");
		this->rasterType = EMPTY;
		return;
	}
	this->rasterPath = rasterFullPath;
	char fileName[200];
	_splitpath(this->rasterPath.c_str(), NULL, NULL, fileName, NULL);
	this->rasterName = fileName;
	this->isDescribed = false;
	this->initializedFromImg = false;
	this->rasterType = rType;
	printf("Raster name: %s\n", this->rasterPath.c_str());
	if (this->rasterType == INPUT)
	{
		if (!this->readRasterProperties())
		{
			std::ifstream f;
			std::string imgFileName = rasterFullPath + ".img";
			f.open(imgFileName.c_str(), std::ios::in);
			if (f.is_open())
			{
				f.close();
				this->convertRasterToFloat();
				this->readRasterProperties();
				this->initializedFromImg = true;
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
		switch (this->rasterType)
		{
		case OUTPUT:
			// Nothing is needed to be created at this stage
			printf("\tInitialized output raster\n");
			break;
		case TEMPORARY:
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
	this->initializedFromImg = false;
	this->rasterType = COPY;
	this->rasterName = g.rasterName;
	this->rasterPath = g.rasterPath;
	g.copyProperties(*this);
}

// Assignment operator overloading
raster & raster::operator = (const raster & g)
{
	initializedFromImg = false;
	if (this != &g)
	{
		this->rasterPath = g.rasterPath;
		this->rasterName = g.rasterName;
		switch (g.rasterType)
		{
		case EMPTY:
			this->rasterType = g.rasterType;
			break;
		default:
			this->rasterType = COPY;
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
		this->convertFloatToRaster();
	}
	if ((this->initializedFromImg && ((this->rasterType == INPUT) || (this->rasterType == OUTPUT)))
		|| (this->rasterType == TEMPORARY))
	{
		this->removeFloatFromDisc();
	}
}

bool raster::isEmpty()
{
	return (this->rasterType == EMPTY);
}

bool raster::readRasterProperties()
{
	std::ifstream f;
	std::string hdrFileName = this->rasterPath + ".hdr";

	int horResolution;
	int verResolution;
	double xMin;
	double yMin;
	double cellSize;

	f.open(hdrFileName.c_str(), std::ios::in);
	if (f.is_open())
	{
		std::string line;
		{
			getline(f, line);
			std::stringstream ss(line);
			ss >> line;
			ss >> horResolution;
		}
		{
			getline(f, line);
			std::stringstream ss(line);
			ss >> line;
			ss >> verResolution;
		}
		{
			getline(f, line);
			replace(line.begin(), line.end(), ',', '.');
			std::stringstream ss(line);
			ss >> line;
			ss >> xMin;
		}
		{
			getline(f, line);
			replace(line.begin(), line.end(), ',', '.');
			std::stringstream ss(line);
			ss >> line;
			ss >> yMin;
		}
		{
			getline(f, line);
			replace(line.begin(), line.end(), ',', '.');
			std::stringstream ss(line);
			ss >> line;
			ss >> cellSize;
		}
		{
			getline(f, line);
			replace(line.begin(), line.end(), ',', '.');
			std::stringstream ss(line);
			ss >> line;
			ss >> this->noDataValue;
		}
		f.close();
		this->extent.init(horResolution, verResolution, xMin, yMin, cellSize);
		return true;
	}
	else
	{
		return false;
	}
}

bool raster::validateExtent(const extentT &otherExtent) const
{
	bool result = (this->extent == otherExtent);
	ASSERT_INT(result, WRONG_EXTENT);
	return result;
}

void raster::saveHdr()
{
	this->extent.saveHdr(this->rasterPath, this->noDataValue);
}

bool raster::fileExists(const std::string & fileName)
{
	std::ifstream ifile(fileName.c_str());
	if (ifile)
	{
		return true;
	}
	return false;
}

void raster::copyFile(const std::string & source, const std::string & destination) const
{
#if defined(_WIN32) || defined(_WIN64) || defined(OS_WINDOWS)
	system((std::string("copy \"") + source + "\" \"" + destination + "\"").c_str());
#else
	system((std::string("cp \"") + source + "\" \"" + destination + "\"").c_str());
#endif
}

void raster::deleteFile(const std::string & fileName) const
{
#if defined(_WIN32) || defined(_WIN64) || defined(OS_WINDOWS)
	std::cout << std::string("del \"") + fileName + "\"" << std::endl;
	system((std::string("del \"") + fileName + "\"").c_str());
#else
	system((std::string("rm \"") + fileName + "\" \"").c_str());
#endif
}

void raster::copyProperties(raster & destination) const
{
	destination.extent = this->extent;
	destination.noDataValue = this->noDataValue;
	destination.isDescribed = this->isDescribed;
	destination.description = this->description;
}

//void raster::incMap(zonalStatisticsTableT &mp, int key, float val)
//{
//	statisticsStructT tmp = mp[key];
//	if (tmp.count == 0)
//	{
//		tmp.sumVal = val;
//		tmp.maxVal = val;
//		tmp.minVal = val;
//		tmp.count = 1;
//		tmp.countNonZero = 1;
//	}
//	else
//	{
//		tmp.sumVal += val;
//		tmp.maxVal = xmax(tmp.maxVal, val);
//		tmp.minVal = xmin(tmp.minVal, val);
//		tmp.count++;
//		tmp.countNonZero += (val > 0) ? 1 : 0;
//	}
//	mp.erase(key);
//	mp.insert(make_pair<int, statisticsStructT>(key, tmp));
//}

raster::statisticsStructT raster::describe()
{
	if (this->isDescribed)
	{
		return this->description;
	}

	printf("Computing statistics\n");

	statisticsStructT result;
	result.count = 0;
	result.maxVal = 0;
	result.minVal = 0;
	result.meanVal = 0;
	result.sumVal = 0;

	BigFileIn thisFile(*this);
	rasterBufT rBuf;	
	
	bool minMaxInitialized = false;

	while (thisFile.read(rBuf))
	{
		for (int i = 0; i < rBuf.nEl; i++)
		{
			if (rBuf.buf[i] != rBuf.noDataValue)
			{
				if (!minMaxInitialized)
				{
					result.maxVal = rBuf.buf[i];
					result.minVal = rBuf.buf[i];
					minMaxInitialized = true;
				}
				result.count++;
				result.sumVal += rBuf.buf[i];
				result.maxVal = xmax(rBuf.buf[i], result.maxVal);
				result.minVal = xmin(rBuf.buf[i], result.minVal);
			}
		}
	}

	result.meanVal = result.sumVal / result.count;

	this->isDescribed = true;
	this->description = result;

	return result;
}

// Geoprocessing methods:
raster raster::copy(const std::string & destinationName)
{
	this->copyFile(this->rasterPath + ".hdr", destinationName + ".hdr");
	this->copyFile(this->rasterPath + ".flt", destinationName + ".flt");
	raster destinationRaster = (*this);
	destinationRaster.rasterPath = destinationName;
	return destinationRaster;
}

void raster::copy(raster & destinationRaster)
{
	std::string destinationName = destinationRaster.rasterPath;
	this->copyFile(this->rasterPath + ".hdr", destinationName + ".hdr");
	this->copyFile(this->rasterPath + ".flt", destinationName + ".flt");
	this->copyProperties(destinationRaster);
}

void raster::removeFloatFromDisc()
{
	deleteFile((this->rasterPath + ".hdr").c_str());
	deleteFile((this->rasterPath + ".flt").c_str());
}

//void raster::rasterArithmetics(float (*func)(float, float), const float num, raster & outRaster)
//{
//	printf("Executing raster arithmetics\n");
//	std::string thisHdrPath = this->rasterPath + ".hdr";
//	std::string thisFltPath = this->rasterPath + ".flt";
//	std::string outHdrPath = outRaster.rasterPath + ".hdr";
//	std::string outFltPath = outRaster.rasterPath + ".flt";
//
//	copyFile(thisHdrPath, outHdrPath);
//	copyProperties(outRaster);
//
//	ifstream thisFile;
//	thisFile.open(thisFltPath.c_str(), ios::in | ios::binary);
//	ofstream outFile;
//	outFile.open(outFltPath.c_str(), ios::out | ios::binary);
//
//	int numCells = (*this).horResolution * (*this).verResolution;
//	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);
//
//	float * buf = new float[bufSize];
//
//	int numCellsProcessed = 0;
//	while(numCellsProcessed < numCells)
//	{
//		bufSize = min(bufSize, numCells - numCellsProcessed);
//		numCellsProcessed += bufSize;
//		thisFile.read(reinterpret_cast<char*>(buf), sizeof(float) * bufSize);
//		for (int i = 0; i < bufSize; i++)
//		{
//			if (buf[i] != (*this).noDataValue)
//			{
//				buf[i] = func(buf[i], num);
//			}
//			else
//			{
//				buf[i] = (*this).noDataValue;
//			}
//		}
//		outFile.write(reinterpret_cast<char *>(buf), sizeof(float) * bufSize);
//		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
//	}
//
//	thisFile.close();
//	outFile.close();
//
//    delete [] buf;
//}
//
//void raster::rasterArithmetics(float (*func)(float, float), const raster & inRaster, raster & outRaster)
//{
//	printf("Executing raster arithmetics\n");
//	validateExtent(inRaster);
//
//	string thisHdrPath = (*this).rasterPath + ".hdr";
//	string thisFltPath = (*this).rasterPath + ".flt";
//	string inHdrPath = inRaster.rasterPath + ".hdr";
//	string inFltPath = inRaster.rasterPath + ".flt";
//	string outHdrPath = outRaster.rasterPath + ".hdr";
//	string outFltPath = outRaster.rasterPath + ".flt";
//
//	copyFile(thisHdrPath, outHdrPath);
//	copyProperties(outRaster);
//
//	ifstream thisFile;
//	thisFile.open(thisFltPath.c_str(), ios::in | ios::binary);
//	ASSERT_INT(thisFile.is_open(), FILE_NOT_OPEN);
//	ifstream inFile;
//	inFile.open(inFltPath.c_str(), ios::out | ios::binary);
//	ASSERT_INT(inFile.is_open(), FILE_NOT_OPEN);
//	ofstream outFile;
//	outFile.open(outFltPath.c_str(), ios::out | ios::binary);
//	ASSERT_INT(outFile.is_open(), FILE_NOT_OPEN);
//
//	int numCells = (*this).horResolution * (*this).verResolution;
//	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);
//
//	float * buf = new float[bufSize];
//	float * buf2 = new float[bufSize];
//
//	int numCellsProcessed = 0;
//	while(numCellsProcessed < numCells)
//	{
//		bufSize = min(bufSize, numCells - numCellsProcessed);
//		numCellsProcessed += bufSize;
//		thisFile.read(reinterpret_cast<char*>(buf), sizeof(float) * bufSize);
//		inFile.read(reinterpret_cast<char*>(buf2), sizeof(float) * bufSize);
//		for (int i = 0; i < bufSize; i++)
//		{
//			if ((buf[i] != (*this).noDataValue) && (buf2[i] != inRaster.noDataValue))
//			{
//				buf[i] = func(buf[i], buf2[i]);
//			}
//			else
//			{
//				buf[i] = (*this).noDataValue;
//			}
//		}
//		outFile.write(reinterpret_cast<char *>(buf), sizeof(float) * bufSize);
//		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
//	}
//
//	thisFile.close();
//	inFile.close();
//	outFile.close();
//
//	delete [] buf;
//	delete [] buf2;
//}

// Conversion methods:
void raster::convertRasterToFloat()
{
	printf("Converting \"%s.img\" to float\n", this->rasterPath.c_str());
	std::string conversionCommand = std::string("convertRasterToFloat.py \"") 
		+ this->rasterPath + ".img\" \"" + this->rasterPath + ".flt\"\n";
	ASSERT_INT(!system(conversionCommand.c_str()), RASTER_TO_FLOAT_CONVERSION_ERROR);
}

void raster::convertFloatToRaster()
{
	printf("Converting \"%s.flt\" to raster\n", this->rasterPath.c_str());
	std::string conversionCommand = std::string("convertFloatToRaster.py \"")
		+ this->rasterPath + ".flt\" \"" + this->rasterPath + ".img\"\n";
	ASSERT_INT(!system(conversionCommand.c_str()), FLOAT_TO_RASTER_CONVERSION_ERROR);
}

std::string raster::getHdrPath() const
{
	return this->rasterPath + ".hdr";
}

std::string raster::getFltPath() const
{
	return this->rasterPath + ".flt";
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