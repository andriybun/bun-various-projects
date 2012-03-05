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
	
	// Map defining the relation between the sum of powers of two 
	// for the rasters present and the probability class
	int * priorMap;

	map<int, vector<int> > tmpAgreementTable;

	void recursive(size_t n, size_t idx, int * vec);
	void computePriorities(const vector<int> & vec, vector<int> & res);
public:
	agreementTableT(const vector<int> & priorityVec1, const vector<int> & priorityVec2);
	agreementTableT(const agreementTableT & g);
	agreementTableT & operator = (const agreementTableT & g);
	~agreementTableT();
	
	int getPriority1(size_t idx);
	int getPriority2(size_t idx);

	int getClass(int sumPowers);
};

#endif