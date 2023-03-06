/**
 * signal() system call example.
 * Copyright (C) 2022 Lazar Razvan.
 *
 * void ( *signal(int sig, void (*handler)(int)) ) (int);
 *
 * signal system call may be implemented in glibc as a library over sigaction()
 * system call.
 *
 * We will use SIGINT (interrupt) signal in the following example.
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "debug.h"

/*============================================================================*/
bool _signaled = false;

/*==================================STATIC====================================*/
/**
 * Implementation for new handler to be used.
 */
static void __test_handler(int signal_no)
{
	_signaled = true;
}

/*============================================================================*/

int main()
{
	void (*old_handler)(int);

	/**
	 * Change SIGNINT signal disposition.
	 *
	 * Note that old SIGINT handler address should be returned on success.
	 */
	old_handler = signal(SIGINT, __test_handler);
	if (old_handler == SIG_ERR) {
		ERROR("signal() error!\n");
		exit(1);
	}

	printf("Waiting to send SIGINT (Ctrl + C) signal...\n");
	while (!_signaled)
		sleep(10);


	printf("SIGINT signal received...\n");

	/**
	 * Ignore SIGINT signal (by the kernel).
	 *
	 * You may use SIGQUIT (Ctrl + \) to stop the process.
	 */
	if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
		ERROR("signal() error!\n");
		exit(2);
	}

	printf("Ignoring SIGINT signal...\n");
	while(true)
		sleep(2);

	return 0;
}
