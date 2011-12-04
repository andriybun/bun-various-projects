#ifndef STATIC_ARRAYS_H_
#define STATIC_ARRAYS_H_

//#define DEBUG_ARRAYS		// uncomment to test exceeding array bounds

#if !defined DEBUG_ARRAYS

	#define array(TYPE, NAME, DIM1)				TYPE NAME[DIM1]
	#define array2d(TYPE, NAME, DIM1, DIM2)		TYPE NAME[DIM1][DIM2]

#else

	#include <assert.h>

	#define array(TYPE, NAME, DIM1)				staticArray<TYPE> NAME(DIM1)
	#define array2d(TYPE, NAME, DIM1, DIM2)		staticArray2d<TYPE> NAME(DIM1, DIM2)

	// Static array 1d
	template <class T>
	class staticArray
	{
	private:
		bool isAllocated;
		int dim1size;
		T * data;
	public:
		staticArray();
		staticArray(int size);
		~staticArray();
		staticArray(const staticArray & orig);
		staticArray & operator = (const staticArray & orig);
		T & operator [] (int idx);
	};

	template <class T>
	staticArray<T>::staticArray()
	{
		isAllocated = false;
		dim1size = 0;
	}

	template <class T>
	staticArray<T>::staticArray(int size)
	{
		dim1size = size;
		data = new T[size];
		isAllocated = true;
	}

	template <class T>
	staticArray<T>::~staticArray(void)
	{
		if (isAllocated) delete [] data;
	}

	template <class T>
	T & staticArray<T>::operator [] (int idx)
	{
		ASSERT_INT(idx >= 0);
		ASSERT_INT(idx < dim1size);
		return data[idx];
	}

	template <class T>
	staticArray<T>::staticArray(const staticArray & orig)
	{
		dim1size = orig.dim1size;
		if (isAllocated) delete [] data;
		data = new T[dim1size];
		memcpy(orig.data, data, dim1size * sizeof(T));
		isAllocated = true;
	}

	template <class T>
	staticArray<T> & staticArray<T>::operator = (const staticArray & orig)
	{
		if (this != &orig)
		{
			dim1size = orig.dim1size;
			if (isAllocated) delete [] data;
			data = new T[dim1size];
			memcpy(orig.data, data, dim1size * sizeof(T));
			isAllocated = true;
		}
		return *this;
	}

	// Static array 2d
	template <class T>
	class staticArray2d
	{
	private:
		int dim0size;
		staticArray<T> * data;
	public:
		staticArray2d(int dim0, int dim1);
		~staticArray2d();
		staticArray<T> & operator [] (int dim0);
	};

	template <class T>
	staticArray2d<T>::staticArray2d(int dim0, int dim1)
	{
		dim0size = dim0;
		data = new staticArray<T>[dim0size];
		for (int i = 0; i < dim0size; i++)
		{
			data[i] = staticArray<T>(dim1);
		}
	}

	template <class T>
	staticArray2d<T>::~staticArray2d(void)
	{
		delete [] data;
	}

	template <class T>
	staticArray<T> & staticArray2d<T>::operator [] (int dim0)
	{
		ASSERT_INT(dim0 >= 0);
		ASSERT_INT(dim0 < dim0size);
		return data[dim0];
	}

#endif

#endif