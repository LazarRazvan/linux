/**
 * Print segments addresses of a process.
 * Copyright (C) 2022 Lazar Razvan.
 *
 * Expose the memory layout of the process by checking the size of diffrent
 * segments
 * 	1) Text segment
 * 		Program machine code instructions
 *
 * 	2) Initialized data segment
 * 		Static and global initialized variables
 *
 * 	3) Uninitialized data segment (.bss)
 * 		Static and global uninitialized variables.
 *
 * The layout is inspected using etext, edata and end variable that track the
 * end of each segment.
 */

#include <stdio.h>

#include "debug.h"

/*============================================================================*/
/**
 * Specific process layout variables.
 */
extern char etext;	// end of text segment
extern char edata;	// end of initialized data segment
extern char end;	// end of uninitialized data segment

/*============================================================================*/

/* Global uninitialized data. (.bss) */
int x, y;

/* Global initialized data. */
int a = 0;
char c = 'b';

/**
 * Test function.
 */
static void __test_function(void)
{
	static int x = 1;	// global initialized data

	DEBUG("Function %s called %d times\n", __func__, x++);
}

/**
 * Function to print process segments.
 */
static void __print_segments(void)
{
	DEBUG("Text end: %p\n", &etext);
	DEBUG("Initialized data end: %p\n", &edata);
	DEBUG("Unnitialized data end: %p\n", &end);
}

int main()
{
	int i = 0;		// local variable (stack)

	for (i = 0; i < 10; i++)
		__test_function();

	__print_segments();

	return 0;
}
