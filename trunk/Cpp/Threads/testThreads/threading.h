#ifndef THREADING_H_
#define THREADING_H_

#include <cmath>

// Uncomment for sequential execution of program in one thread
//#define SEQUENTIAL_EXECUTION

#ifndef SEQUENTIAL_EXECUTION
	#if defined(_WIN32) || defined(_WIN64) || defined(OS_WINDOWS)
		#include <windows.h>
		#define PARALLEL_ON_WINDOWS
	#else
		// Write smth here for non windows OS'es

	#endif
#else
	// Write smth here for non parallel execution

#endif

template <class inDataT, class outDataT>
struct exchangeT
{
	int count;
	inDataT * inData;
	outDataT * outData;
	int (*execute)(inDataT &, outDataT &);
};

template <class inDataT, class outDataT>
#ifdef PARALLEL_ON_WINDOWS
	DWORD WINAPI ThreadFun(LPVOID lpParam)
#else
	unsigned long ThreadFun(void * lpParam)
#endif
{
	exchangeT<inDataT, outDataT> * localDataPtr = (exchangeT<inDataT, outDataT> *)lpParam;

	for (int i = 0; i < localDataPtr->count; i++)
	{
		localDataPtr->execute(localDataPtr->inData[i], localDataPtr->outData[i]);
	}

	return 0;
}

template <class inDataT, class outDataT>
int parallelExecute(int (*execute)(inDataT &, outDataT &), inDataT * arr, outDataT * outArr, int count)
{
#ifdef PARALLEL_ON_WINDOWS
	SYSTEM_INFO sysinfo;
	GetSystemInfo( &sysinfo );
	int numCpus = sysinfo.dwNumberOfProcessors;
	printf("Detected %d CPU's\n", numCpus);

	int NUM_THREADS = numCpus;
	HANDLE * hThread = new HANDLE[NUM_THREADS];
	DWORD * threadID = new DWORD[NUM_THREADS];
	exchangeT<inDataT, outDataT> * msgArray = new exchangeT<inDataT, outDataT>[NUM_THREADS];

	// Distribute array between threads
	int itemsPerThread = (int)ceil((float)count / NUM_THREADS);
	for (int i = 0; i < NUM_THREADS; i++)
	{
		msgArray[i].count = min(itemsPerThread, count - i * itemsPerThread);
		msgArray[i].inData = &arr[i * itemsPerThread];
		msgArray[i].outData = &outArr[i * itemsPerThread];
		msgArray[i].execute = execute;
	}

	DWORD threadsCreated = 0;
	for (int i = 0; i < NUM_THREADS; i++)
	{
		hThread[i] = CreateThread(NULL, // security attributes ( default if NULL )
			0, // stack SIZE default if 0
			ThreadFun<inDataT, outDataT>, // Start Address
			&msgArray[i], // input data
			0, // creational flag ( start if  0 )
			&threadID[i]); // thread ID
		if (!hThread[i])
		{
			printf("Thread #%d is not created!\n", i+1);
		}
		else
		{
			threadsCreated++;
		}
	}

	if (WaitForMultipleObjects(threadsCreated, hThread, true, INFINITE) == (DWORD)0xFFFFFFFF)
	{
		printf("Error finishing threads: %d\n", GetLastError());
	}

	for (int i = 0; i < NUM_THREADS; i++)
	{
		CloseHandle(hThread[i]);
	}

	// Cleanup:
	delete [] hThread;
	delete [] threadID;
	delete [] msgArray;
#else
	int NUM_THREADS = 1;
	exchangeT<inDataT, outDataT> msgArray;

	msgArray.count = count;
	msgArray.inData = arr;
	msgArray.outData = outArr;
	msgArray.execute = execute;

	ThreadFun<inDataT, outDataT>(&msgArray);
#endif

	return 0;
}

#endif