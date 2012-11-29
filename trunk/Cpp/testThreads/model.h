#ifndef MODEL_H_
#define MODEL_H_

#include <cmath>

#include "intf.h"

#define min(x, y) ((x)<(y) ? (x):(y))
#define max(x, y) ((x)<(y) ? (y):(x))

template <class inDataT, class outDataT>
int modelFunc(inDataT &inData, outDataT &outData)
{
	int tmp = inData;
	for (int i = 0; i < 100000; i++)
	{
		tmp = inData * 8;
		outData = tmp / 8;
	}

	outData = inData; // stopgap
	return 0;
}

#endif