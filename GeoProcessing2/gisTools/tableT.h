#ifndef TABLET_H_
#define TABLET_H_

#include <iostream>
#include <cstdlib>
#include <map>
#include <vector>

#include "assertInt.h"
#include "errorCodes.h"

class raster;

class tableT
{
public:
	typedef std::map< int, std::vector<float> > dataT;
	dataT data;
private:
	bool sized;
	size_t numCols;
public:
	tableT();
	tableT(size_t sz);
	~tableT();
	void setNumCols(size_t n);
	void insert(const int key, const std::vector<float> & val);
	void inc(const int key, const std::vector<float> & val);
	void inc(const int key, const size_t idx, const float val);
	dataT::iterator find(const std::vector<float> & val);
	bool exists(const std::vector<float> & val);
	size_t size();

	// Friends:
	friend class raster;
};

#endif