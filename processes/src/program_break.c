/**
 * Program break (heap segment) example.
 * Copyright (C) 2022 Lazar Razvan.
 *
 * Expose the behavior of program break (top of heap data segment) when working
 * with heap memory specific functions (brk(); sbrk(); malloc(); free())
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "debug.h"

/*============================================================================*/

/* Initialized in first step */
static void *_base_program_break = NULL;

/* Allocations number */
#define ALLOC_NUM					1000

/* Allocation mem size */
#define BLK_SIZE					2048

/* Buffer to track allocated addresses */
void *mem[ALLOC_NUM];

/*==================================STATIC====================================*/
/**
 * Initialize program break (heap segment).
 */
static inline void __init_program_break(void)
{
	_base_program_break = sbrk(0);
}

/**
 * Get base program break (heap segment) address using uninitialized data
 * segment address.
 */
static inline void * __get_base_program_break(void)
{
	return _base_program_break;
}

/**
 * Get current program break (heap segment) address using sbrk.
 */
static inline void * __get_curr_program_break(void)
{
	return (void *)sbrk(0);
}

/**
 * Print base program break address.
 */
static inline void __print_base_program_break(void)
{
	DEBUG("[%s] Program break = %p\n", __func__, __get_base_program_break());
}

/**
 * Print current program break address.
 */
static inline void __print_curr_program_break(void)
{
	DEBUG("[%s] Program break = %p\n", __func__, __get_curr_program_break());
}

/**
 * Print heap segment size.
 */
static inline void __print_heap_size(void)
{
	DEBUG("[%s] Heap size = %ld bytes\n", __func__,
						__get_curr_program_break() -__get_base_program_break());
}

/**
 * Print heap info.
 */
static inline void __print_heap_info(void)
{
	__print_base_program_break();
	__print_curr_program_break();
	__print_heap_size();
}

/*===================================TESTS====================================*/

/**
 * Perform ALLOC_NUM malloc allocation of BLK_SIZE bytes.
 *
 * After allocation is finished, inspect program break address which has to be
 * increased due to heap allocations.
 */
static void __malloc_test(void)
{
	for (int i = 0; i < ALLOC_NUM; i++) {
		mem[i] = malloc(BLK_SIZE);
		if (!mem[i]) {
			ERROR("malloc error!\n");
			exit(1);
		}
	}
}

/**
 * Dump address of each memory block allocated and check the size difference
 * with the next one. It can be seen that the memory between two blocks in a
 * row it is not exactly BLK_SIZE bytes since metadata is placed before
 * address returned by malloc (including block size).
 *
 *  -----------------------------------------------------
 *  | Length   |           Block memory                  |
 *  |  (L)     |                                         |
 *  -----------------------------------------------------
 *             |
 *        Address returned by malloc.
 */
static void __dump_test(void)
{
	unsigned char *_b;

	for (int i = 0; i < ALLOC_NUM; i++) {
		DEBUG("Block[%d]\n", i);
		DEBUG("    Starting address = %p\n", mem[i]);

		// Bytes between two consecutive blocks
		if (i != ALLOC_NUM - 1)
			DEBUG("    Dif to next blck = %ld\n", mem[i+1] - mem[i]);

		// Hexdump memory before address returned by malloc
		if (i) {
			_b = (unsigned char *)(mem[i-1] + BLK_SIZE);
			for (; _b != mem[i]; ) {
				printf("%02X ", *_b);
				_b++;
			}

			printf("\n");
		}
	}
}

/**
 * Free ALLOC_NUM blocks of memory of BLK_SIZE bytes.
 *
 * As it can be seen in the output, program break is not decreased with every
 * free call, but instead the block is inserted into a free list to be reused
 * in a further step (if necessary). For a freed block, its metadata track
 * beside the length of the block, two pointers, one to the next free block
 * available and one to the previous block available.
 *
 *  ---------------------------------------------------------------
 *  | Length   | Previous free blk | Next free blk |  Blk memory   |
 *  |  (L)     |       (P)         |    (N)        |               |
 *  ---------------------------------------------------------------
 */
static void __free_test(void)
{
	void *_b, *_a;

	for (int i = 0; i < ALLOC_NUM; i++) {
		printf("Freeing block %d\n", i);

		_b = __get_curr_program_break();
		free(mem[i]);
		_a = __get_curr_program_break();

		if (_b == _a)
			printf("    Program break not changed!\n");
		else
			printf("    Program break CHANGED!\n");
	}
}

/*============================================================================*/

int main()
{
	/* Initialize program break (before performing any heap allocation) */
	__init_program_break();

	/**
	 * Note that kernel most of the time allocate memory as multiple of page
	 * size.
	 */
	printf("Page size = %ld\n", sysconf(_SC_PAGE_SIZE));

	/**
	 * Note that printf implementation use a heap buffer for print, this is the
	 * reason that program break address is increased without us altering heap
	 * memory. This is why, after first print, reinitialize base program break.
	 */
	__print_heap_info();

	/* Move after printf buffer allocated on heap */
	printf("===============================================================\n");
	__init_program_break();
	__print_heap_info();

	/* Allocate memory */
	printf("===============================================================\n");
	printf("%d mallocs of %d bytes\n", ALLOC_NUM, BLK_SIZE);
	__malloc_test();
	__print_heap_info();

	/* Inspect memory */
	printf("===============================================================\n");
	__dump_test();

	/* Free memory */
	printf("===============================================================\n");
	printf("%d free of %d bytes\n", ALLOC_NUM, BLK_SIZE);
	__free_test();
	__print_heap_info();


	return 0;
}
