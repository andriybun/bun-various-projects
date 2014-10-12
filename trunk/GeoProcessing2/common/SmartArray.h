#pragma once

#include <boost\shared_array.hpp>

template<class T>
class SmartArray final
{
	size_t n;
	typedef boost::shared_array<T> smartArrayT;
	smartArrayT data;

	bool isAllocated;
public:
	SmartArray(void);
	SmartArray(const size_t n);
	SmartArray(const SmartArray&) = default;
	SmartArray& operator=(const SmartArray&) = default;
	T& operator[] (const size_t i);

	void allocate(const size_t n);
	void allocateOnce(const size_t n);
	T& operator* ();
	T* operator-> ();
	T* ptr();
};