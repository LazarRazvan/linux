/**
 * Open system call example.
 * Copyright (C) 2022 Lazar Razvan.
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "debug.h"

extern int errno;

/*=================================== FILES ==================================*/

#define FILE_NOT_EXISTING			"file_not_existing.txt"
#define FILE_EXISTING				"file_existing.txt"
#define FILE_EXISTING_RO			"file_existing_ro.txt"

/*=================================== TESTS ==================================*/
#define TESTS_NO		6

/**
 * Test 0 : Open not existing file.
 *
 * This result in error unless O_CREAT flag is used as in Test 1.
 */
static void test0(void)
{
	int fd;

	fd = open(FILE_NOT_EXISTING, O_RDWR);
	if (fd == -1) {
		ERROR("%s!\n", strerror(errno));
		return;
	}
}

/**
 * Test 1 : Open not existing file.
 *
 * Success since O_CREAT flag is used so missing file will be created.
 */
static void test1(void)
{
	int fd;

	fd = open(FILE_NOT_EXISTING, O_RDWR | O_CREAT);
	if (fd == -1) {
		ERROR("%s!\n", strerror(errno));
		return;
	}

	if (close(fd) == -1) {
		ERROR("%s!\n", strerror(errno));
		return;
	}
}

/**
 * Test 2: Open read only file in read-write mode.
 *
 * This result in an error since user does not have write permissions for this
 * file.
 */
static void test2(void)
{
	int fd;

	fd = open(FILE_EXISTING_RO, O_RDWR);
	if (fd == -1) {
		ERROR("%s!\n", strerror(errno));
		return;
	}

	if (close(fd) == -1) {
		ERROR("%s!\n", strerror(errno));
		return;
	}
}

/**
 * Test 3: Open (and create) file with read and write permissions for current
 * user.
 *
 * Note that if exists, mode flags are not taken into account. 
 */
static void test3(void)
{
	int fd;
	mode_t mode = 0;

	mode |= S_IRUSR;	// user read permissions
	mode |= S_IWUSR;	// user write permissions

	fd = open("user_rw.txt", O_RDWR | O_CREAT, mode);
	if (fd == -1) {
		ERROR("%s!\n", strerror(errno));
		return;
	}

/**
 * [razvan@fedora io]$ ls -la
 * -rw-------. 1 razvan razvan     0 Sep 23 07:55 user_rw.txt
 */

	if (close(fd) == -1) {
		ERROR("%s!\n", strerror(errno));
		return;
	}
}

/**
 * Test 4: Open (and create) file with read and write permissions for current
 * user and group;
 *
 * Note that if exists, mode flags are not taken into account. 
 */
static void test4(void)
{
	int fd;
	mode_t mode = 0;

	mode |= S_IRUSR;	// user read permissions
	mode |= S_IWUSR;	// user write permissions
	mode |= S_IRGRP;	// group read permissions
	mode |= S_IWGRP;	// group write permissions

	fd = open("user_group_rw.txt", O_RDWR | O_CREAT, mode);
	if (fd == -1) {
		ERROR("%s!\n", strerror(errno));
		return;
	}

/**
 * [razvan@fedora io]$ ls -la
 * -rw-rw----. 1 razvan razvan     0 Sep 23 07:55 user_group_rw.txt
 */

	if (close(fd) == -1) {
		ERROR("%s!\n", strerror(errno));
		return;
	}
}

/**
 * Test 5: Open (and create) file with read and write permissions for current
 * user, group and others;
 *
 * Note that if exists, mode flags are not taken into account. 
 */
static void test5(void)
{
	int fd;
	mode_t mode = 0;

	mode |= S_IRUSR;	// user read permissions
	mode |= S_IWUSR;	// user write permissions
	mode |= S_IRGRP;	// group read permissions
	mode |= S_IWGRP;	// group write permissions
	mode |= S_IROTH;	// other read permissions
	mode |= S_IWOTH;	// other write permissions

	fd = open("user_group_other_rw.txt", O_RDWR | O_CREAT, mode);
	if (fd == -1) {
		ERROR("%s!\n", strerror(errno));
		return;
	}

/**
 * [razvan@fedora io]$ ls -la
 * -rw-rw-rw-. 1 razvan razvan     0 Sep 23 07:55 user_group_other_rw.txt
 */

	if (close(fd) == -1) {
		ERROR("%s!\n", strerror(errno));
		return;
	}
}
/*================================= TESTS LIST ===============================*/
typedef void (*_list)(void);

_list tests_list[TESTS_NO] = {
	test0,
	test1,
	test2,
	test3,
	test4,
	test5,
};

int main(int argc, char *argv[])
{
	int test_no;

	// Read test number
	printf("\nEnter test number <0-%d>!\n", TESTS_NO - 1);
	scanf("%d", &test_no);

	// Validate test number
	if (test_no < 0 || test_no > TESTS_NO - 1) {
		ERROR("Invalid test number!\n");
		return -1;
	}

	// Call test
	tests_list[test_no]();

	return 0;
}
