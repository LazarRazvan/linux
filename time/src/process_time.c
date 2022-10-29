/**
 * Process time measuring example.
 * Copyright (C) 2022 Lazar Razvan.
 *
 * Measure the process time using times() function call. This is a simple
 * implementation that use two struct tms structures to get the start and end
 * process time. There is also an init flag, since API is not reentrant.
 *
 * Time is measured in units called "clock ticks". To obtain number of clock
 * ticks per seconds "sysconf(_SC_CLK_TCK)" can be used.
 */

#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/times.h>

#include "debug.h"

/*============================================================================*/
/**
 * Max number of timers.
 */
#define MAX_TIMERS			8

/**
 * Structure to track timers.
 */
typedef struct _ptime {

	char 		init;		// check if timer is initialized (start method)
	char		used;		// check if timer is used (register method)
	struct tms	start;		// track start time
	struct tms	end;		// track end time

} ptime;

/*============================================================================*/

/**
 * Global timers array.
 */
ptime tm[MAX_TIMERS] = { 0 };

/**
 * Global timers state.
 */
char init_state = 0;

/*================================= STATIC ===================================*/

/**
 * Get free timer.
 */
static inline int __timer_alloc(void)
{
	for (int i = 0; i < MAX_TIMERS; i++)
		if (tm[i].used == 0)
			return i;

	return -1;
}

/**
 * Free timer.
 */
static inline void __timer_free(int i)
{
	tm[i].used = 0;
}

/**
 * Validate timer descriptor.
 */
static inline int __timer_fd_validate(int timer_fd)
{
	if (timer_fd < 0 || timer_fd >= MAX_TIMERS)
		return 1;

	return 0;
}
/*================================= PUBLIC ===================================*/

/**
 * Initialize process time.
 */
void process_time_init(void)
{
	// Initi timers state
	init_state = 1;

	// Init timers array
	for (int i = 0; i < MAX_TIMERS; i++) {
		tm[i].init = 0;
		tm[i].used = 0;
	}
}

/**
 * Register timer for process time measuring.
 *
 * Return timer descriptor on success (>= 0) and <0 on error.
 */
int process_time_register(void)
{
	int timer_fd;

	// Check if process_time_init was previously called
	if (!init_state) {
		ERROR("timers not initialized\n");
		return -1;
	}

	// Lookup for available timer
	timer_fd = __timer_alloc();
	if (timer_fd < 0) {
		ERROR("all timers in used!\n");
		return -2;
	}

	tm[timer_fd].init = 0;
	tm[timer_fd].used = 1;

	return timer_fd;
}

/**
 * Start measuring process time for a given timer.
 *
 * @timer_fd : Timer descriptor.
 *
 * Return 0 on success and print time, or <0 on error.
 *
 * Errors:
 * 	1) Invalid timer descriptor
 * 	2) Timer not registered
 */
int process_time_start(int timer_fd)
{
	// Check if process_time_init was previously called
	if (!init_state) {
		ERROR("timers not initialized\n");
		return -1;
	}

	// Validate timer descriptor.
	if (__timer_fd_validate(timer_fd)) {
		ERROR("Invalid timer %d\n", timer_fd);
		return -2;
	}

	// Check if timer was previously register
	if (tm[timer_fd].used == 0) {
		ERROR("Timer %d not previously registered!\n", timer_fd);
		return -3;
	}

	// Get start
	if (times(&tm[timer_fd].start) == -1) {
		ERROR("times error!\n");
		return -4;
	}

	tm[timer_fd].init = 1;

	return 0;
}

/**
 * End measuring process time for a given timer.
 *
 * @timer_fd : Timer descriptor.
 *
 * Return 0 on success and print time, or <0 on error.
 *
 * Errors:
 * 	1) Invalid timer descriptor
 * 	2) Timer not registered
 * 	3) Timer not initialized
 */
int process_time_end(int timer_fd)
{
	long clk_ticks_per_sec = sysconf(_SC_CLK_TCK);

	// Check if process_time_init was previously called
	if (!init_state) {
		ERROR("timers not initialized\n");
		return -1;
	}

	// Validate timer descriptor.
	if (__timer_fd_validate(timer_fd)) {
		ERROR("Invalid timer %d\n", timer_fd);
		return -2;
	}

	// Check if timer was previously register
	if (tm[timer_fd].used == 0) {
		ERROR("Timer %d not previously registered!\n", timer_fd);
		return -3;
	}

	// Check if start was previously called
	if (tm[timer_fd].init == 0) {
		ERROR("Timer %d not started!\n", timer_fd);
		return -4;
	}

	// Get end
	if (times(&tm[timer_fd].end) == -1) {
		ERROR("times error!\n");
		return -5;
	}

	// Print time
	printf("user CPU time: %.3f\n",
		(double)(tm[timer_fd].end.tms_utime - tm[timer_fd].start.tms_utime) /
		clk_ticks_per_sec);

	printf("sys CPU time: %.3f\n",
		(double)(tm[timer_fd].end.tms_stime - tm[timer_fd].start.tms_stime) /
		clk_ticks_per_sec);

	return 0;
}

/**
 * Release timer for process time measuring.
 *
 * @timer_fd	: Timer descriptor.
 *
 * Return 0 on success and <0 otherwise.
 *
 * Errors:
 * 	1) Invalid timer descriptor
 * 	2) Timer not previously registered
 */
int process_time_release(int timer_fd)
{
	// Check if process_time_init was previously called
	if (!init_state) {
		ERROR("timers not initialized\n");
		return -1;
	}

	// Validate timer descriptor.
	if (__timer_fd_validate(timer_fd)) {
		ERROR("Invalid timer %d\n", timer_fd);
		return -2;
	}

	// Check if timer was previously register
	if (tm[timer_fd].used == 0) {
		ERROR("Timer %d not previously registered!\n", timer_fd);
		return -3;
	}

	// Release timer
	__timer_free(timer_fd);

	return 0;
}
