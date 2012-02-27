#ifndef AGREEMENT_TABLE_
#define AGREEMENT_TABLE_

#include <vector>
#include <map>

using namespace std;

class agreementTableT
{
private:
	vector<int> prior1;
	vector<int> prior2;
	size_t numRasters;
	size_t numClasses;
	int ** priorMap;
	map<int, vector<int> > tmpAgreementTable;

	void recursive(size_t n, size_t idx, int * vec);
	void computePriorities(const vector<int> & vec, vector<int> & res);
public:
	agreementTableT(vector<int> & priorityVec1, vector<int> & priorityVec2);
	~agreementTableT();
	
	int getPriority1(size_t idx);
	int getPriority2(size_t idx);
};

#endif