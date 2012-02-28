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
	recursive(numRasters, 0, vec);
	// Process preliminary agreement table
	map<int, vector<int> >::iterator it = tmpAgreementTable.begin();
	int cl = 0;
	while (it != tmpAgreementTable.end())
	{
		priorMap[it->second[3]] = cl++;
		it++;
	}
	delete [] vec;
}

agreementTableT::~agreementTableT()
{
	delete [] priorMap;
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

void agreementTableT::computePriorities(const vector<int> & vec, vector<int> & res)
{
	vector<int> tmp = vec;
	vector<int> newTmp;
	newTmp.resize(vec.size());
	res.resize(vec.size());
	map<int, int> oldNewMapping;
	sort(tmp.begin(), tmp.end());
	int pr = 1;
	int count = 1;
	newTmp[0] = pr;
	oldNewMapping.insert(make_pair<int, int>(tmp[0], newTmp[0]));
	for (size_t idx = 1; idx < vec.size(); idx++)
	{
		if (tmp[idx] != tmp[idx-1])
		{
			pr = count + 1;
		}
		newTmp[idx] = pr;
		count += pr;
		oldNewMapping.insert(make_pair<int, int>(tmp[idx], newTmp[idx]));
	}
	for (size_t idx = 0; idx < vec.size(); idx++)
	{
		res[idx] = oldNewMapping[vec[idx]];
	}
}

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