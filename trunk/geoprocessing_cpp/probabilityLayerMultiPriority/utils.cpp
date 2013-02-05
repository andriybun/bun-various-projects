#include <algorithm>

#include "utils.h"
#include "assertInt.h"
#include "errorCodes.h"

int initializePriorityData(string csvPriorityFile, int numRasters, map<int, priorityDataT * > &priorityDataMap)
{
	//////////////////////////
	// Parse csv file
	//////////////////////////
	const char field_terminator = ',';
	const char line_terminator  = '\n';
	const char enclosure_char   = '"';

	csv_parser file_parser;

	/* Define how many records we're gonna skip. This could be used to skip the column definitions. */
	file_parser.set_skip_lines(1);

	/* Specify the file to parse */
	file_parser.init(csvPriorityFile.c_str());

	/* Here we tell the parser how to parse the file */
	file_parser.set_enclosed_char(enclosure_char, ENCLOSURE_OPTIONAL);

	file_parser.set_field_term_char(field_terminator);

	file_parser.set_line_term_char(line_terminator);

	unsigned int row_count = 1U;

	printf("======================================\n");
	printf("\tCSV contents\n");
	printf("======================================\n");

	/* Check to see if there are more records, then grab each row one at a time */
	while(file_parser.has_more_rows())
	{
		unsigned int i = 0;

		// Get the record
		csv_row row = file_parser.get_row();

		// Check if number of columns in a row is the same as number of rasters
		ASSERT_INT(row.size() == 3 * numRasters + 1, WRONG_CSV_FILE);

		priorityDataT * priorityDataTmp = new priorityDataT;
		priorityDataTmp->prioritiesVector.resize(numRasters);
		priorityDataTmp->prioritiesVector2.resize(numRasters);
		priorityDataTmp->weightsVector.resize(numRasters);

		vector<double> tmpPrior1;
		tmpPrior1.resize(numRasters);

		for (i = 0; i < (unsigned int)numRasters; i++)
		{
			tmpPrior1[i] = atof(row[1 + 0 * numRasters + i].c_str()); // priorityDataTmp->prioritiesVector[i]
			priorityDataTmp->prioritiesVector2[i] = atoi(row[1 + 1 * numRasters + i].c_str());
			priorityDataTmp->weightsVector[i] = atoi(row[1 + 2 * numRasters + i].c_str());
		}

		// Convert vector of double to vector of integers ordered by doubles' values
		agreementTableT::computePriorities(tmpPrior1, priorityDataTmp->prioritiesVector);

		// Initialize agreement table
		priorityDataTmp->agTable = new agreementTableT(priorityDataTmp->prioritiesVector, priorityDataTmp->prioritiesVector2);

		printf("%d \t%f \t%f\n", atoi(row[0].c_str()), tmpPrior1[0], tmpPrior1[1]);
		priorityDataMap.insert(pair<int, priorityDataT * >(atoi(row[0].c_str()), priorityDataTmp));

		row_count++;
	}
	
	// Add a virtual country with id NO_VALIDATION_POINTS_COUNTRY_ID for countries thad didn't have validation points
	priorityDataT * priorityDataTmp = new priorityDataT;
	priorityDataTmp->prioritiesVector.resize(numRasters);
	priorityDataTmp->prioritiesVector2.resize(numRasters);
	priorityDataTmp->weightsVector.resize(numRasters);

	for (unsigned int i = 0; i < (unsigned int)numRasters; i++)
	{
		priorityDataTmp->prioritiesVector[i] = 1;
		priorityDataTmp->prioritiesVector2[i] = 1;
		priorityDataTmp->weightsVector[i] = 1;
	}

	// Initialize agreement table
	priorityDataTmp->agTable = new agreementTableT(priorityDataTmp->prioritiesVector, priorityDataTmp->prioritiesVector2);

	printf("%d \t%d \t%d\n", NO_VALIDATION_POINTS_COUNTRY_ID, 1, 1);
	priorityDataMap.insert(pair<int, priorityDataT * >(NO_VALIDATION_POINTS_COUNTRY_ID, priorityDataTmp));

	printf("======================================\n");

	return 0;
}

int destroyPriorityData(map<int, priorityDataT * > &priorityDataMap)
{
	map<int, priorityDataT * >::iterator it;

	for (it = priorityDataMap.begin() ; it != priorityDataMap.end(); it++)
	{
		delete (it->second)->agTable;
		delete it->second;
	}

	return 0;
}

void adjustCroplandProbabilityLayer(raster & inAreaRaster,
									raster & inCountriesRaster,
									raster & inClassRaster,
									raster & outClassRaster,
									const runParamsT & params,
									map<int, priorityDataT * > & priorityDataMap
									)
{
	printf("Adjusting cropland probability classes\n");

	map< float, float * > classesByCountryTable;	// key - country ID, value - vector of areas for all classes
	map< float, vector<int> > oldToNewClassesMap;	// key - country ID, value - vector of new (adjusted) classes

	inAreaRaster.validateExtent(inCountriesRaster);
	inAreaRaster.validateExtent(inClassRaster);

	raster::statisticsStructT probabilityStatistics = inClassRaster.describe();
	size_t maxClass = (size_t)probabilityStatistics.maxVal;
	size_t minClass = (size_t)probabilityStatistics.minVal;

	vector<int> dummyVector(maxClass+1, -999);

	ifstream inAreaFile;
	inAreaFile.open(inAreaRaster.getFltPath().c_str(), ios::in | ios::binary);
	ASSERT_INT(inAreaFile.is_open(), FILE_NOT_OPEN);
	ifstream inCountriesFile;
	inCountriesFile.open(inCountriesRaster.getFltPath().c_str(), ios::in | ios::binary);
	ASSERT_INT(inCountriesFile.is_open(), FILE_NOT_OPEN);
	ifstream inClassFile;
	inClassFile.open(inClassRaster.getFltPath().c_str(), ios::in | ios::binary);
	ASSERT_INT(inClassFile.is_open(), FILE_NOT_OPEN);
	ofstream outClassFile;
	outClassFile.open(outClassRaster.getFltPath().c_str(), ios::out | ios::binary);
	
	inAreaRaster.copyFile(inAreaRaster.getHdrPath(), outClassRaster.getHdrPath());
	inAreaRaster.copyProperties(outClassRaster);

	long countriesBegin = inCountriesFile.tellg();
	long classBegin = inClassFile.tellg();

	int numCells = inAreaRaster.horResolution * inAreaRaster.verResolution;
	int bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);

	float * bufArea = new float[bufSize];
	float * bufCountries = new float[bufSize];
	float * bufClass = new float[bufSize];
	float * outBufClass = new float[bufSize];

	printf("Computing statistics for classes per country\n");
	int numCellsProcessed = 0;

	// First run - collecting statistics
	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		numCellsProcessed += bufSize;

		inAreaFile.read(reinterpret_cast<char*>(bufArea), sizeof(float) * bufSize);
		inCountriesFile.read(reinterpret_cast<char*>(bufCountries), sizeof(float) * bufSize);
		inClassFile.read(reinterpret_cast<char*>(bufClass), sizeof(float) * bufSize);

		for (int i = 0; i < bufSize; i++)
		{
			if ((bufClass[i] != inClassRaster.noDataValue) &&
				(bufCountries[i] != inCountriesRaster.noDataValue) &&
				(bufArea[i] != inAreaRaster.noDataValue))
			{
				map< float, float * >::iterator it = classesByCountryTable.find(bufCountries[i]);
				if (it == classesByCountryTable.end())
				{
					printf("  > adding: c = %d\n", (int)bufCountries[i]);
					float * tmp = new float[maxClass + 1];
					memset(tmp, 0, sizeof(float) * (maxClass + 1));
					classesByCountryTable.insert(make_pair<float, float *>(bufCountries[i], tmp));
					oldToNewClassesMap.insert(make_pair<float, vector<int> >(bufCountries[i], dummyVector));
					it = classesByCountryTable.find(bufCountries[i]);
				}
				it->second[(int)bufClass[i]] += bufArea[i];
			}
		}
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}
	
	// Second run through the table - rearrange classes
	map< float, float * >::iterator it = classesByCountryTable.begin();
	while (it != classesByCountryTable.end())
	{
		float countryId = it->first;

		for (size_t i = 0; i <= maxClass; i++)
		{
			oldToNewClassesMap[countryId][i] = -1;
		}

		for (size_t i = minClass; i <= maxClass; i++)
		{
			oldToNewClassesMap[countryId][i] = (oldToNewClassesMap[countryId][i] < 0) ? (int)i : oldToNewClassesMap[countryId][i];

			float swp = it->second[i];
			int swpClass = i;
			bool swpFl = false;

			size_t j = i + 1;

			map<int, priorityDataT * >::iterator countryPriorityData = priorityDataMap.find(countryId);

			// If the country ID isn't found in list, use unit priorities for all rasters
			if (countryPriorityData == priorityDataMap.end())
			{
				countryPriorityData = priorityDataMap.find(NO_VALIDATION_POINTS_COUNTRY_ID);
			}
			agreementTableT * agTable = countryPriorityData->second->agTable;

			while ((j <= maxClass) && agTable->checkSimilarity(i, j))
			{
				if (it->second[j] > swp)
				{
					swp = it->second[j];
					swpClass = j;
					swpFl = true;
				}
				j++;
			}

			if (swpFl)
			{
				it->second[swpClass] = it->second[i];
				it->second[i] = swp;
				int subs = oldToNewClassesMap[countryId][i];
				oldToNewClassesMap[countryId][i] = (oldToNewClassesMap[countryId][swpClass] < 0)
					? (int)swpClass
					: oldToNewClassesMap[countryId][swpClass];
				oldToNewClassesMap[countryId][swpClass] = subs;
			}

		}
	
		it++;
	}

	// Third run - write adjusted classes into the resulting raster
	inCountriesFile.seekg(countriesBegin);
	inClassFile.seekg(classBegin);
	
	printf("Writing results to the output file\n");
	numCellsProcessed = 0;
	bufSize = xmin(numCells, MAX_READ_BUFFER_ELEMENTS);

	while(numCellsProcessed < numCells)
	{
		bufSize = min(bufSize, numCells - numCellsProcessed);
		numCellsProcessed += bufSize;

		inCountriesFile.read(reinterpret_cast<char*>(bufCountries), sizeof(float) * bufSize);
		inClassFile.read(reinterpret_cast<char*>(bufClass), sizeof(float) * bufSize);

		for (int i = 0; i < bufSize; i++)
		{
			if ((bufClass[i] != inClassRaster.noDataValue) &&
				(bufCountries[i] != inCountriesRaster.noDataValue))
			{
				outBufClass[i] = (float)oldToNewClassesMap[bufCountries[i]][(int)bufClass[i]];
			}
			else
			{
				outBufClass[i] = outClassRaster.noDataValue;
			}
		}
		outClassFile.write(reinterpret_cast<char *>(outBufClass), sizeof(float) * bufSize);
		printf("%5.2f%% processed\n", (float)100 * numCellsProcessed / numCells);
	}	

	// Free up memory
	delete [] bufArea;
	delete [] bufCountries;
	delete [] bufClass;
	delete [] outBufClass;

	it = classesByCountryTable.begin();
	while (it != classesByCountryTable.end())
	{
		delete [] it->second;
		it++;
	}

	classesByCountryTable.clear();
	inAreaFile.close();
	inCountriesFile.close();
	inClassFile.close();
}