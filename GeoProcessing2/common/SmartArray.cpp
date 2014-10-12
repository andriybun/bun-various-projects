#include "SmartArray.h"

template<class T>
SmartArray<T>::SmartArray(void)
{
	this->isAllocated = false;
	this->data = smartArrayT(new T);
};

template<class T>
SmartArray<T>::SmartArray(const size_t n)
{
	this->n = n;
	this->data = smartArrayT(new T[n]);
	this->isAllocated = true;
};

template<class T>
void SmartArray<T>::allocate(size_t n)
{
	if (this->isAllocated)
	{
		throw "SmartArray is already allocated";
	}
	this->isAllocated = true;
	this->n = n;
	this->data.reset(new T[n]);
}

template<class T>
void SmartArray<T>::allocateOnce(size_t n)
{
	if ((!this->isAllocated) || (this->n < n))
	{
		this->isAllocated = true;
		this->n = n;
		this->data.reset(new T[n]);
	}
}

template<class T>
T& SmartArray<T>::operator* ()
{
	return *(this->data.get());
}

template<class T>
T* SmartArray<T>::operator-> ()
{
	return this->data.get();
}

template<class T>
T* SmartArray<T>::ptr()
{
	return this->data.get();
}

template<class T>
T& SmartArray<T>::operator[] (const size_t i)
{
	return this->data[i];
}

template class SmartArray<float>;