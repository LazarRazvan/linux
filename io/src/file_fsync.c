/**
 * Linux reduce I/O buffering using fsync().
 * Copyright (C) 2022 Lazar Razvan.
 *
 * Expose how file system and kernel deals with read() and write() system calls
 * when file data and metadata are forced to be updated to disk using fsync()
 * call.
 *
 * fsync is responsible for synchronized I/O file integrity completion. This
 * means that all files data and metadata (timestamp, file size, etc ...) are
 * written to disk and only after fsync() call returns.
 *
 * Expose the differences between "file_buffering" where time penalty is related
 * to multiple read()/write() system calls.
 *
 * Using fsync() after each write() call will increase significantly execution
 * time, since that is not buffer but instead is written to disk.
 *
 * Examples of other related functions: fdatasync() and sync(). Also, the same
 * behavior can be achieved using O_SYNC flag when opening destination file.
 *
 * Use 100K as destination file for example.
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>

#include "debug.h"
#include "process_time.h"

extern int errno;

/*============================================================================*/
/**
 * Copy data from source file to destination file.
 *
 * @buf		: Buffer to be used for moving data.
 * @buf_size: Size of the buffer.
 * @fd_src	: Source file descriptor.
 * @fd_dst	: Destination file descriptor.
 *
 * Return 0 on success and <0 otherwise.
 */
static int __copy(void *buf, unsigned int buf_size, int fd_src, int fd_dst)
{
	int timer_fd, rv = 0;
	ssize_t bytes_r, bytes_w;

	/**
	 * Create and start timer.
	 */
	timer_fd = process_time_register();
	if (timer_fd < 0) {
		ERROR("Fail to register timer!\n");
		rv = -1; goto end;
	}

	if (process_time_start(timer_fd) < 0) {
		ERROR("Fail to start timer!\n");
		rv = -2; goto end;
	}

	/**
	 * Loop to read from source file until reach end-of-file and write data
	 * to destination file.
	 */
	while (1) {
		bytes_r = read(fd_src, buf, buf_size);
		if (bytes_r < 0) {
			ERROR("%s!\n", strerror(errno));
			rv = -3; goto end;
		}

		if (!bytes_r)
			break;	// source end-of-file reached

		bytes_w = write(fd_dst, buf, bytes_r);
		if (bytes_w < 0) {
			ERROR("%s!\n", strerror(errno));
			rv = -4; goto end;
		}

		if (bytes_r != bytes_w) {
			ERROR("Fail to write data!\n");
			rv = -5; goto end;
		}

		/**
		 * For synchronized I/O file integrity completion.
		 */
		if (fsync(fd_dst)) {
			ERROR("Fsync error!\n");
			rv = -6; goto end;
		}
	}

	/**
	 * Stop and release timer.
	 */
	if (process_time_end(timer_fd) < 0) {
		ERROR("Fail to stop timer!\n");
		rv = -7; goto end;
	}

	if (process_time_release(timer_fd)) {
		ERROR("Fail to release timer!\n");
		rv = -8; goto end;
	}

end:
	return rv;
}

/*============================================================================*/

int main(int argc, char *argv[])
{
	void *buf;
	mode_t mode;
	char *src, *dst;
	int fd_src, fd_dst, rv = 0;
	unsigned int buf_size;

	/**
	 * Validate arguments.
	 */
	if (argc != 3) {
		ERROR("Invalid format: ./my_cp <source> <destination>\n");
		rv = -1; goto end;
	}

	src = argv[1];
	dst = argv[2];

	/**
	 * Init timers.
	 */
	process_time_init();

	/**
	 * Main loop.
	 */
	for (int i = 0; i < 14; i++) {

		/**********************************************************************
		 * Files open.
		 * 1) src: mandatory to exist (open in read-only mode)
		 * 2) dst: create if doesn't exist (open in write-only mode) (rw-rw-rw-)
		 **********************************************************************/
		fd_src = open(src, O_RDONLY);
		if (fd_src == -1) {
			ERROR("%s!\n", strerror(errno));
			rv = -2; goto end;
		}

		mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
		fd_dst = open(dst, O_WRONLY|O_CREAT, mode);
		if (fd_dst == -1) {
			if (close(fd_src) == -1) {
				ERROR("%s!\n", strerror(errno));
				rv = -3; goto end;
			}
			ERROR("%s!\n", strerror(errno));
			rv = -3; goto end;
		}

		/***********************************************************************
		 * Create buffer size, alloc buffer and move data.
		 **********************************************************************/
		buf_size = (2 << i);
		printf("Running with buffer_size = %u\n", buf_size);

		buf = malloc(buf_size);
		if (!buf) {
			ERROR("malloc failed!\n");
			assert(close(fd_src) != -1);
			assert(close(fd_dst) != -1);
			rv = -4; goto end;
		}

		if (__copy(buf, buf_size, fd_src, fd_dst)) {
			ERROR("copy failed!\n");
			free(buf);
			assert(close(fd_src) != -1);
			assert(close(fd_dst) != -1);
			rv = -5; goto end;
		}

		free(buf);

		/***********************************************************************
		 * Close files.
		 **********************************************************************/
		if (close(fd_src) == -1) {
			ERROR("%s!\n", strerror(errno));
			assert(close(fd_dst) != -1);
			rv = -5; goto end;
		}

		if (close(fd_dst) == -1) {
			ERROR("%s!\n", strerror(errno));
			rv = -6; goto end;
		}
	}

end:
	return rv;
}
