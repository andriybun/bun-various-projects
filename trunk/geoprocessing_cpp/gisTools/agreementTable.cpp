#include <cmath>
#include <algorithm>

#include "assertInt.h"
#include "errorCodes.h"

#include "agreementTable.h"

agreementTableT::agreementTableT(const vector<int> & priorityVec1, const vector<int> & priorityVec2)
{
	computePriorities(priorityVec1, prior1);
	computePriorities(priorityVec2, prior2);
	numRasters = prior1.size();
	int * vec = new int[numRasters];
	numClasses = (size_t)(2 << (numRasters - 1)); // pow((double)2, (double)numRasters);
	priorMap = new int[numClasses];
	similarClassesMatrix = new bool * [numClasses];
	for (size_t idx = 0; idx < numClasses; idx++)
	{
		similarClassesMatrix[idx] = new bool[numClasses];
		for (size_t j = 0; j < numClasses; j++)
		{
			similarClassesMatrix[idx][j] = false;
		}
	}
	recursive(numRasters, 0, vec);
	// Process preliminary agreement table
	map<int, vector<int> >::iterator it = tmpAgreementTable.begin();
	int cl = 0;
	while (it != tmpAgreementTable.end())
	{
		priorMap[it->second[3]] = cl;
		map<int, vector<int> >::iterator itFwd = it;
		itFwd++;
		int clFwd = cl + 1;
		similarClassesMatrix[cl][cl] = true;
		while (itFwd != tmpAgreementTable.end())
		{
			if ((it->second[1] == itFwd->second[1]) && (it->second[2] == itFwd->second[2]))
			{
				similarClassesMatrix[cl][clFwd] = similarClassesMatrix[clFwd][cl] = true;
			}
			clFwd++;
			itFwd++;
		}
		cl++;
		it++;
	}
	delete [] vec;
}

agreementTableT::agreementTableT(const agreementTableT & g)
{
	prior1 = g.prior1;
	prior2 = g.prior2;
	numRasters = g.numRasters;
	numClasses = g.numClasses;
	tmpAgreementTable = g.tmpAgreementTable;
	priorMap = new int[numClasses];
	for (size_t i = 0; i < numClasses; i++)
	{
		priorMap[i] = g.priorMap[i];
	}
	similarClassesMatrix = new bool * [numClasses];
	for (size_t idx = 0; idx < numClasses; idx++)
	{
		similarClassesMatrix[idx] = new bool[numClasses];
		for (size_t j = 0; j < numClasses; j++)
		{
			similarClassesMatrix[idx][j] = g.similarClassesMatrix[idx][j];
		}
	}
}

agreementTableT & agreementTableT::operator = (const agreementTableT & g)
{
	if (this != &g)
	{
		prior1 = g.prior1;
		prior2 = g.prior2;
		numRasters = g.numRasters;
		numClasses = g.numClasses;
		tmpAgreementTable = g.tmpAgreementTable;
		priorMap = new int[numClasses];
		for (size_t i = 0; i < numClasses; i++)
		{
			priorMap[i] = g.priorMap[i];
		}
		similarClassesMatrix = new bool * [numClasses];
		for (size_t idx = 0; idx < numClasses; idx++)
		{
			similarClassesMatrix[idx] = new bool[numClasses];
			for (size_t j = 0; j < numClasses; j++)
			{
				similarClassesMatrix[idx][j] = g.similarClassesMatrix[idx][j];
			}
		}
	}
}

agreementTableT::~agreementTableT()
{
	delete [] priorMap;
	for (size_t idx = 0; idx < numClasses; idx++)
	{
		delete [] similarClassesMatrix[idx];
	}
	delete [] similarClassesMatrix;
}

void agreementTableT::recursive(size_t n, size_t idx, int * vec)
{
	if (idx < n)
	{
		for (size_t i = 0; i < 2; i++)
		{
			vec[idx] = i;
			recursive(n, idx+1, vec);
		}
	}
	else
	{
		vector<int> sum;
		sum.resize(4);
		sum[0] = 0; // number of rasters agree
		sum[1] = 0; // sum of 1st priorities
		sum[2] = 0; // sum of 2nd priorities
		sum[3] = 0; // sum of 2 to the power of raster's index. Needed to identify the unique sum
		for (size_t i = 0; i < n; i++)		
		{
			sum[0] += vec[i];
			sum[1] += vec[i] * prior1[i];
			sum[2] += vec[i] * prior2[i];
			sum[3] += vec[i] * (2 << i) / 2;
		}
		tmpAgreementTable.insert(pair<int, vector<int> >((sum[1] * numClasses + sum[2]) * numClasses + sum[3], sum));
	}
}

template <class T>
void agreementTableT::computePriorities(const vector<T> & vec, vector<int> & res)
{
	vector<T> tmp = vec;
	vector<int> newTmp;
	newTmp.resize(vec.size());
	res.resize(vec.size());
	map<T, int> oldNewMapping;
	sort(tmp.begin(), tmp.end());
	int pr = 1;
	int count = 1;
	newTmp[0] = pr;
	oldNewMapping.insert(make_pair<T, int>(tmp[0], newTmp[0]));
	for (size_t idx = 1; idx < vec.size(); idx++)
	{
		if (tmp[idx] != tmp[idx-1])
		{
			pr = count + 1;
		}
		newTmp[idx] = pr;
		count += pr;
		oldNewMapping.insert(make_pair<T, int>(tmp[idx], newTmp[idx]));
	}
	for (size_t idx = 0; idx < vec.size(); idx++)
	{
		res[idx] = oldNewMapping[vec[idx]];
	}
}

template void agreementTableT::computePriorities<int>(const vector<int> & vec, vector<int> & res);
template void agreementTableT::computePriorities<double>(const vector<double> & vec, vector<int> & res);

int agreementTableT::getPriority1(size_t idx)
{
	ASSERT_INT((idx >= 0) && (idx < numRasters), ARRAY_INDEX_OUT_OF_BOUNDS);
	return prior1[idx];
}

int agreementTableT::getPriority2(size_t idx)
{
	ASSERT_INT((idx >= 0) && (idx < numRasters), ARRAY_INDEX_OUT_OF_BOUNDS);
	return prior2[idx];
}

int agreementTableT::getClass(int sumPowers)
{
	return priorMap[sumPowers];
}

bool agreementTableT::checkSimilarity(size_t class1, size_t class2)
{
	return similarClassesMatrix[class1][class2];
}