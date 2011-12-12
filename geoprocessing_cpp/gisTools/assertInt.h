#ifndef ASSERT_INT_H_
#define ASSERT_INT_H_

#define ASSERT_INT(ARG, ERROR_CODE)											\
	{																		\
	if (!(ARG))															\
		{																	\
		fprintf(stderr, __TIME__ "\n");									\
		fprintf(stderr, "File: " __FILE__ " \tline: %d\n", __LINE__);	\
		fprintf(stderr, "Assertion failed: " #ARG "\n");				\
		exit(ERROR_CODE);												\
		}																	\
	}

#endif