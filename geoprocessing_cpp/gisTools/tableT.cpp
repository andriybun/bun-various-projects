#include "tableT.h"

// tableT subclass:
tableT::tableT()
{
	sized = false;
}

tableT::tableT(size_t sz)
{
	setNumCols(sz);
}

tableT::~tableT()
{

}

void tableT::setNumCols(size_t n)
{
	ASSERT_INT(!sized, OTHER_ERROR);
	numCols = n;
	sized = true;
}

void tableT::insert(const float key, const vector<float> & val)
{
	ASSERT_INT(val.size() == numCols, OTHER_ERROR);
	data.insert(make_pair<float, vector<float>>(key, val));
}

void tableT::inc(const float key, const vector<float> & val)
{
	ASSERT_INT(val.size() == numCols, OTHER_ERROR);
	vector<float> tmp;
	tmp = data[key];
	for (size_t idx = 0; idx < numCols; idx++)
	{
		tmp[idx] += val[idx];
	}
	data.erase(key);
	data.insert(make_pair<float, vector<float>>(key, tmp));
}

void tableT::inc(const float key, const size_t idx, const float val)
{
	ASSERT_INT(idx < numCols + 1, OTHER_ERROR);
	vector<float> tmp = data[key];
	if (tmp.size() == 0)
	{
		tmp.resize(numCols);
	}
	tmp[idx-1] += val;
	data.erase(key);
	data.insert(make_pair<float, vector<float>>(key, tmp));
}

tableT::dataT::iterator tableT::find(const vector<float> & val)
{
	dataT::iterator it = data.begin();
	while (it != data.end())
	{
		if (it->second == val) return it;
		it++;
	}
	return it;
}

bool tableT::exists(const vector<float> & val)
{
	return find(val) != data.end();
}

size_t tableT::size()
{
	return data.size();
}
