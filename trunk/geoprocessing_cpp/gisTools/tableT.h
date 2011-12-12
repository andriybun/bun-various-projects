#ifndef TABLET_H_
#define TABLET_H_

// tableT class for tables
class tableT
{
public:
	typedef map< float, vector<float> > dataT;
private:
	dataT data;
	bool sized;
	size_t numCols;
public:
	tableT();
	tableT(size_t sz);
	~tableT();
	void setNumCols(size_t n);
	void insert(const float key, const vector<float> & val);
	void inc(const float key, const vector<float> & val);
	void inc(const float key, const size_t idx, const float val);
	// TODO: optimize
	dataT::iterator find(const vector<float> & val);
	bool exists(const vector<float> & val);
	size_t size();
	friend class raster;
};
// end of tableT class

#endif