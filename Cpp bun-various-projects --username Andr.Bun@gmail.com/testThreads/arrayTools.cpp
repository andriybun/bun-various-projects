#include "arrayTools.h"

template <typename T>
void initVector(T *vec, int size, T (*initFunc)(int))
{
	for (int i = 0; i < size; i++)
	{
		vec[i] = (*initFunc)(i);
	}
}

template <typename T>
void printVector(T * vec, int size)
{
    for (int i = 0; i < size; i++)
    {
        cout << vec[i] << endl;
    }
}

template void initVector<int>(int *vec, int size, int (*initFunc)(int));
template void initVector<float>(float *vec, int size, float (*initFunc)(int));
template void initVector<double>(double *vec, int size, double (*initFunc)(int));

template void printVector<int>(int * vec, int size);
template void printVector<float>(float * vec, int size);
template void printVector<double>(double * vec, int size);
