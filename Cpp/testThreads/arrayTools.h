#ifndef ARRAYTOOLS_H_
#define ARRAYTOOLS_H_

#include <stdio.h>
#include <iostream>

using namespace std;

template <typename T>
void initVector(T *vec, int size, T (*initFunc)(int));

template <typename T>
void printVector(T *vec, int size);

#endif