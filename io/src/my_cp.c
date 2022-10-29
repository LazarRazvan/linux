/**
 * Linux cp command basic implementation.
 * Copyright (C) 2022 Lazar Razvan.
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "debug.h"

extern int errno;

/*============================================================================*/
/* Buffer size */
#define BUF_SIZE		1024

/*============================================================================*/
/**
 * Copy data from source file to destination file.
 *
 * @fd_src	: Source file descriptor.
 * @fd_dst	: Destination file descriptor.
 *
 * Return 0 on success and <0 otherwise.
 */
static int __copy(int fd_src, int fd_dst)
{
	int rv = 0;
	void *buf = NULL;
	ssize_t bytes_r, bytes_w;

	buf = malloc(BUF_SIZE);
	if (!buf) {
		ERROR("Fail to allocate buffer!\n");
		rv = -1; goto end;
	}

	/**
	 * Loop to read from source file until reach end-of-file and write data
	 * to destination file.
	 */
	while (1) {
		bytes_r = read(fd_src, buf, BUF_SIZE);
		if (bytes_r < 0) {
			ERROR("%s!\n", strerror(errno));
			rv = -2; goto end;
		}

		if (!bytes_r)
			break;	// source end-of-file reached

		bytes_w = write(fd_dst, buf, bytes_r);
		if (bytes_w < 0) {
			ERROR("%s!\n", strerror(errno));
			rv = -3; goto end;
		}

		if (bytes_r != bytes_w) {
			ERROR("Fail to write data!\n");
			rv = -4; goto end;
		}
	}

end:
	return rv;
}

/*============================================================================*/

int main(int argc, char *argv[])
{
	mode_t mode;
	char *src, *dst;
	int fd_src, fd_dst, rv = 0;

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
	 * Files open.
	 * 1) src: mandatory to exist (open in read-only mode)
	 * 2) dst: create if doesn't exist (open in write-only mode) (rw-rw-rw-)
	 */
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

	/**
	 * Copy data from source to destination.
	 */
	if (__copy(fd_src, fd_dst)) {
		rv = -4; goto end;
	}

	/**
	 * Close files.
	 */
	if (close(fd_src) == -1) {
		ERROR("%s!\n", strerror(errno));
		rv = -5; goto end;
	}

	if (close(fd_dst) == -1) {
		ERROR("%s!\n", strerror(errno));
		rv = -6; goto end;
	}
	
end:
	return rv;
}
