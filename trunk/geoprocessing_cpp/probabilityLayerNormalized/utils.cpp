#include "utils.h"
#include "assertInt.h"
#include "errorCodes.h"

int initializePriorityData(char * argv[], int numRasters, priorityDataT * &priorityData)
{
	// Assert if the pointer already points to a data
	ASSERT_INT(priorityData == NULL, MEMORY_ALREADY_ALLOCATED);

	// Allocate memory
	priorityData = new priorityDataT;
	priorityData->prioritiesVector.resize(numRasters);
	priorityData->prioritiesVector2.resize(numRasters);
	priorityData->weightsVector.resize(numRasters);

	// Initialize priority vectors
	for (size_t idx = 0; idx < (size_t)numRasters; idx++)
	{
		priorityData->prioritiesVector[idx] = atoi(argv[idx]);
		priorityData->prioritiesVector2[idx] = atoi(argv[numRasters * 1 + idx]);
		priorityData->weightsVector[idx] = atoi(argv[numRasters * 2 + idx]);
	}

	// Initialize agreement table
	priorityData->agTable = new agreementTableT(priorityData->prioritiesVector, priorityData->prioritiesVector2);

	return 0;
}
