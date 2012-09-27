#include <iostream>
#include <vector>

#include "raster.h"
#include "timer.h"

int main(int argc, char * argv[])
{
	ASSERT_INT(argc == NUM_INPUT_PARAMS, INCORRECT_INPUT_PARAMS);

	// Command line arguments:
	// 1 - 

	printf("Start: ");
	outputLocalTime();
	Timer timer;
	timer.start();

	//////////////////////////////
	//                          //
	//  Insert your code here   //
	//                          //
	//////////////////////////////

	printf("End: ");
	outputLocalTime();
	timer.stop();
	printf("Elapsed time: %5.2f seconds.\n", timer.elapsedSeconds());

	return 0;
}