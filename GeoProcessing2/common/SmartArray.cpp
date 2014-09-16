#include "SmartArray.h"

template<class T>
void SmartArray<T>::allocate(size_t n)
{
	if (this->isAllocated)
	{
		throw "SmartArray is already allocated";
	}
	this->isAllocated = true;
	this->reset(new T[n]);
}

template<class T>
T& SmartArray<T>::operator* ()
{
	return *(this->get());
}

template<class T>
T* SmartArray<T>::operator-> ()
{
	return this->get();
}

template<class T>
T* SmartArray<T>::ptr()
{
	return this->get();
}

template class SmartArray<float>;

//template<class T>
//SmartArray<T>::SmartArray(void)
//{
//	this->isAllocated = false;
//}
//
//template<class T>
//SmartArray<T>::SmartArray(size_t n)
//{
//	this->allocate(n);
//}
//
//template<class T>
//SmartArray<T>::SmartArray(const SmartArray<T>& other)
//{
//	this->data = other.data;
//	this->refCount = other.refCount;
//	this->isAllocated = other.isAllocated;
//	(*(this->refCount))++;
//}
//
//template<class T>
//SmartArray<T>& SmartArray<T>::operator = (const SmartArray<T>& rhs)
//{
//	if (this != &rhs)
//	{
//		// Decrement the old reference count
//		// if reference become zero delete the old data
//		(*(this->refCount))--;
//		if(this->refCount == 0)
//		{
//			this->deallocate();
//		}
//
//		// Copy the data and reference pointer
//		// and increment the reference count
//		this->data = rhs.data;
//		this->refCount = rhs.refCount;
//		this->isAllocated = rhs.isAllocated;
//	}
//	return *this;
//}
//
//template<class T>
//SmartArray<T>::~SmartArray(void)
//{
//	this->deallocate();
//}
//
//template<class T>
//void SmartArray<T>::allocate(size_t n)
//{
//	this->data = new T[n];
//	this->refCount = new size_t;
//	*(this->refCount) = 0;
//}
//
//template<class T>
//void SmartArray<T>::deallocate()
//{
//	delete [] this->data;
//	delete this->refCount;
//}
//
//template<class T>
//T& SmartArray<T>::operator* ()
//{
//	return *(this->data);
//}
//
//template<class T>
//T SmartArray<T>::operator[] (size_t i)
//{
//	if (!isAllocated)
//	{
//		throw "SmartArray not allocated";
//	}
//	return this->data[i];
//}
//
//template<class T>
//T* SmartArray<T>::operator-> ()
//{
//	return this->data;
//}
//
//template<class T>
//T* SmartArray<T>::ptr()
//{
//	return this->data;
//}
//
