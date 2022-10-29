/**
 * Linux I/O buffering.
 * Copyright (C) 2022 Lazar Razvan.
 *
 * Expose how file system and kernel deals with read() and write() system calls.
 *
 * When performing the write() system call, data is not directly written to disk
 * (which is a slow operation) but instead it is cached into a "buffer cache" in
 * kernel. Whenever a process wants to read data from file, it is rather passed
 * from "buffer cache" then reading from disk.
 *
 * When performing the read() system call, data is copied from disk to "buffer
 * cache" in kernel. Most of the time, data is read in advanced to be prepared
 * for later read() calls.
 *
 * Based on the above, this program expose the time involved for copying a file
 * using different size for the buffer used to read data. While the buffer size
 * increase, the elapsed time decrease. This is because the number of system
 * call decrease, and rather not due to file disk operations, since most of the
 * time, data is cached.
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
	}

	/**
	 * Stop and release timer.
	 */
	if (process_time_end(timer_fd) < 0) {
		ERROR("Fail to stop timer!\n");
		rv = -6; goto end;
	}

	if (process_time_release(timer_fd)) {
		ERROR("Fail to release timer!\n");
		rv = -7; goto end;
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
