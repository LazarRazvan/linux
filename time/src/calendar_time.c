/**
 * Program break (heap segment) example.
 * Copyright (C) 2022 Lazar Razvan.
 *
 * Expose usage of calendar time measuring the time since Epoch (1 Jan 1970) and
 * conversions to broken down time (either UTC or system local time).
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>		// sleep
#include <time.h>		// time
#include <sys/time.h>	// gettimeofday

#include "debug.h"

/*============================================================================*/

#define BUF_SIZE		128

char time_buf[BUF_SIZE];

/*============================================================================*/

/**
 * Print struct tm data structure.
 *
 * @_tm	: Time data structure.
 */
static void __print_tm(struct tm *_tm)
{
	DEBUG("[%s] tm_sec = %d\n", __func__, _tm->tm_sec);
	DEBUG("[%s] tm_min = %d\n", __func__, _tm->tm_min);
	DEBUG("[%s] tm_hour = %d\n", __func__, _tm->tm_hour);
	DEBUG("[%s] tm_mday = %d\n", __func__, _tm->tm_mday);
	DEBUG("[%s] tm_mon = %d\n", __func__, _tm->tm_mon);
	DEBUG("[%s] tm_year = %d\n", __func__, _tm->tm_year);
	DEBUG("[%s] tm_wday = %d\n", __func__, _tm->tm_wday);
	DEBUG("[%s] tm_yday = %d\n", __func__, _tm->tm_yday);
	DEBUG("[%s] tm_isdst = %d\n", __func__, _tm->tm_isdst);
}


/**
 * Convert time_t data structure to broken down time corresponding to UTC.
 *
 * @t		: Input data structure to be converted.
 * @gm_time	: Output data structure filled by function.
 */
static void __timet_to_gmtime(time_t *t, struct tm **gm_time)
{
	*gm_time = gmtime(t);
	if (!*gm_time) {
		ERROR("gmtime error!\n");
		exit(1);
	}

	DEBUG("[gmtime] ... \n");
	__print_tm(*gm_time);
}

/**
 * Convert time_t data structure to broken down time corresponding to system
 * local time.
 *
 * @t			: Input data structure to be converted.
 * @local_time	: Output data structure filled by function.
 */
static void __timet_to_localtime(time_t *t, struct tm **local_time)
{
	*local_time = localtime(t);
	if (!*local_time) {
		ERROR("localtime error!\n");
		exit(1);
	}

	DEBUG("[localtime] ... \n");
	__print_tm(*local_time);
}

/*============================================================================*/

int main()
{
	size_t bytes;
	char *buf;
	time_t cal_time;
	struct tm *gm_time, *local_time;

	/**
	 * Use time() function to return calendar time. gettimeofday() function
	 * may be also used as an alternative.
	 */
	cal_time = time(NULL);
	if (cal_time == -1) {
		ERROR("time error!\n");
		return -1;
	}

	/**
	 * Use ctime() function to print calendar time as human readable. Function
	 * takes an time_t structure as input.
	 */
	buf = ctime(&cal_time);
	if (!buf) {
		ERROR("ctime error!\n");
		return -2;
	}

	DEBUG("[cmtime] = %s", buf);

	/**
	 * Convert time to UTC broken down time.
	 */
	__timet_to_gmtime(&cal_time, &gm_time);

	/**
	 * Convert time to system local broken down time.
	 */
	__timet_to_localtime(&cal_time, &local_time);

	/**
	 * Use asctime to print locatime and gmtime to human readable formats.
	 */
	buf = asctime(gm_time);
	if (!buf) {
		ERROR("asctime error!\n");
		return -3;
	}

	DEBUG("[asctime][gmtime] = %s", buf);

	buf = asctime(local_time);
	if (!buf) {
		ERROR("asctime error!\n");
		return -3;
	}

	DEBUG("[asctime][localtime] = %s", buf);

	/**
	 * Custom format for gmtime print.
	 *
	 * - 4-digit year
	 * - full week day name
	 * - full month name
	 * - 12 hour clock
	 * - AM/PM
	 */
	bytes = strftime(time_buf, BUF_SIZE, "%Y %A %B %I %p", gm_time);
	if (bytes == 0) {
		ERROR("strftime error!\n");
		return -4;
	}

	DEBUG("[strftime][gmtime] = %.*s\n", (int)bytes, time_buf);

	return 0;
}
