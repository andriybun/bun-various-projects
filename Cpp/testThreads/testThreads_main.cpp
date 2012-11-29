#include <stdio.h>

#include <iostream>

#include "arrayTools.h"
#include "timer.h"
#include "threading.h"

#include "intf.h"
#include "model.h"

using namespace std;

int main()
{
	Timer stopWatch;

	int inArr[MAX_ARRAY_SIZE];
	int outArr[MAX_ARRAY_SIZE];

	for(int i = 0; i < MAX_ARRAY_SIZE; i++)
	{
		inArr[i] = i + 1 + i * i;
	}

	stopWatch.start();

	parallelExecute<int, int>(&modelFunc<int, int>, inArr, outArr, MAX_ARRAY_SIZE);

	stopWatch.stop();
	printf("Elapsed time is %5.2e seconds\n", stopWatch.elapsedSeconds());

}
