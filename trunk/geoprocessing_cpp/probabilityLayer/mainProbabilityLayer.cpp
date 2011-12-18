#include <iostream>
#include <vector>
// additional include directory: $(PYTHON_INCLUDE)
//#include <Python.h>

#include "raster.h"
#include "timer.h"

int main(int argc, char * argv[])
{
	//ASSERT_INT(argc == 11, INCORRECT_INPUT_PARAMS);

	// Command line arguments:
	// 1 - workingDir
	// 2 - resultDir
	// 3 - tmpDir
	// 4 - 

	printf("Start: ");
	outputLocalTime();
	Timer timer;
	timer.start();

	runParamsT runParams;
	runParams.workingDir = string(argv[1]) + "\\";
	runParams.resultDir = string(argv[2]) + "\\";
	runParams.tmpDir = string(argv[3]) + "\\";

	

	printf("End: ");
	outputLocalTime();
	timer.stop();
	printf("Elapsed time: %5.2f seconds.\n", timer.elapsedSeconds());

	return 0;
}