#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

/**
 * Config: Enable/Disable macros
 */
#define DEBUG_ENABLE		1
#define ERROR_ENABLE		1

// Debug macro
#if DEBUG_ENABLE
#	define DEBUG(fmt, ...) 		\
		printf("%s:%d:debug::"fmt, __func__, __LINE__, ##__VA_ARGS__)
#else
#	define DEBUG(fmt, ...)	{}
#endif

// Error macro
#if ERROR_ENABLE
#	define ERROR(fmt, ...) 		\
		printf("%s:%d:error::"fmt, __func__, __LINE__, ##__VA_ARGS__)
#else
#	define ERROR(fmt, ...)	{}
#endif

#endif	// DEBUG_H

