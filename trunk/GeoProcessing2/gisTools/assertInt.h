#pragma once

#include "timer.h"

#define ASSERT_INT(ARG, ERROR_CODE)											\
	{																		\
	if (!(ARG))																\
		{																	\
			outputLocalTime();												\
			fprintf(stderr, "File: " __FILE__ " \tline: %d\n", __LINE__);	\
			fprintf(stderr, "Assertion failed: " #ARG "\n");				\
			system("pause");												\
			exit(ERROR_CODE);												\
		}																	\
	}