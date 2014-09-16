#pragma once

#include <boost\shared_array.hpp>

template<class T>
class SmartArray : public boost::shared_array<T>
{
	bool isAllocated;
public:
	//using boost::shared_array<T>::shared_array;
	SmartArray(void) : boost::shared_array<T>(new T)
	{
		this->isAllocated = false;
	}; 
	SmartArray(const size_t n) : boost::shared_array<T>(new T[n])
	{
		this->isAllocated = true;
	};

	void allocate(const size_t n);
	T& operator* ();
	T* operator-> ();
	T* ptr();
};

//class SmartArray
//{
//	bool isAllocated;
//	size_t* refCount;
//	T* data;
//
//	void deallocate();
//public:
//	SmartArray(void);
//	SmartArray(const size_t n);
//	SmartArray(const SmartArray& other);
//	SmartArray& operator=(const SmartArray& rhs);
//	virtual ~SmartArray(void);
//
//	void allocate(const size_t n);
//	T& operator* ();
//	T operator[] (size_t i);
//	T* operator-> ();
//	T* ptr();
//};
