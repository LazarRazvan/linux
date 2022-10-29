/**
 * Test program for process_time implementation.
 * Copyright (C) 2022 Lazar Razvan.
 *
 * Expose the implementation by creating a large array on heap and fill in each
 * value (system CPU time intensive) and sorting the array using bubble sort
 * mechanism (user CPU time intensive).
 *
 * 1) array sort that is usermode intensive
 * 2) TODO
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "debug.h"
#include "process_time.h"

/*============================================================================*/

/**
 * Global buffer to be used for sorting.
 */
#define BUF_SIZE		(2 * 4096)

/*============================================================================*/

/**
 * Sort buffer (bubble sort).
 *
 * Note that complexity for bubble sort is often O(n*n). You should not use this
 * in practice. It is choose for now to measure intense user mode process time.
 */
static void __buf_sort(int *v)
{
	int temp, sort_timer_fd;

	// Register buffer creation timer and start measuring time
	sort_timer_fd = process_time_register();
	if (sort_timer_fd < 0) {
		ERROR("Fail to register buffer sort timer!\n");
		return;
	}
	DEBUG("Buffer sort timer_fd = %d\n", sort_timer_fd);

	if (process_time_start(sort_timer_fd) < 0) {
		ERROR("Fail to start buffer sort timer!\n");
		return;
	};

	for (int i = 0; i < BUF_SIZE - 1; i++) {
		for (int j = 0; j < BUF_SIZE - i - 1; j++) {
			if (v[j] > v[j+1]) {
				temp = v[j];
				v[j] = v[j+1];
				v[j+1] = temp;
			}
		}
	}
	
	// Stop timer and release 
	DEBUG("Buffer sort timer!\n");
	if (process_time_end(sort_timer_fd) < 0) {
		ERROR("Fail to stop buffer sort timer!\n");
		free(v);
		return;
	}

	if (process_time_release(sort_timer_fd)) {
		ERROR("Fail to release buffer sort timer!\n");
		free(v);
		return;
	}
}

/**
 * Alloc and fill in a buffer of BUF_SIZE elements.
 */
static void __buf_create(void)
{
	int *v = NULL;
	int create_timer_fd;

	// Register buffer creation timer and start measuring time
	create_timer_fd = process_time_register();
	if (create_timer_fd < 0) {
		ERROR("Fail to register buffer create timer!\n");
		return;
	}
	DEBUG("Buffer create timer_fd = %d\n", create_timer_fd);

	if (process_time_start(create_timer_fd) < 0) {
		ERROR("Fail to start buffer create timer!\n");
		return;
	};

	/**
	 * Create buffer
	 *
	 * Note that we use getpid to generate elements so that a syscall is
	 * performed (this only work for glibc versions >= 2.25 when cached pid
	 * value was removed).
	 */
	srand(time(NULL));

	v = calloc(BUF_SIZE, sizeof(int));
	assert(v);

	for (int i = 0; i < BUF_SIZE; i++)
		v[i] = getpid() + rand();

	// Stop timer and release 
	DEBUG("Buffer create timer!\n");
	if (process_time_end(create_timer_fd) < 0) {
		ERROR("Fail to stop buffer create timer!\n");
		free(v);
		return;
	}

	if (process_time_release(create_timer_fd)) {
		ERROR("Fail to release buffer create timer!\n");
		free(v);
		return;
	}

	// Sort the buffer
	__buf_sort(v);
}

/*============================================================================*/

int main()
{
	int main_timer_fd;

	// Init process time for measuring program time
	process_time_init();

	// Register main timer and start measuring time
	main_timer_fd = process_time_register();
	if (main_timer_fd < 0) {
		ERROR("Fail to register main timer!\n");
		return -1;
	}
	DEBUG("Main timer_fd = %d\n", main_timer_fd);

	if (process_time_start(main_timer_fd) < 0) {
		ERROR("Fail to start main timer!\n");
		return -2;
	}


	// Buffer creation and sort
	__buf_create();

	// Stop main timer and release
	DEBUG("Main timer!\n");
	if (process_time_end(main_timer_fd) < 0) {
		ERROR("Fail to stop main timer!\n");
		return -3;
	}

	if (process_time_release(main_timer_fd)) {
		ERROR("Fail to release main timer!\n");
		return -4;
	}

	return 0;
}
