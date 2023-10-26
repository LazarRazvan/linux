/**
 * Create a new process using fork().
 * Copyright (C) 2023 Lazar Razvan.
 *
 * Expose the mechanism of creating a new process using fork() system call.
 *
 * When using fork() the new process is almost a duplicate of the parent
 * process. Child process text, data, heap and stack segnments are duplicated
 * using copy-on-write mechanism, and new page entries is allocated only when
 * child attempts to alter a page.
 *
 * Note that child process is not guaranteed to be scheduled to CPU before the
 * parent.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "debug.h"


/*============================================================================*/
/* Global initialized data. */
static int global_data = 10;


/*============================================================================*/

int main()
{
	pid_t cpid;
	int local_data = 20;

	/**
	 * Create a child process. fork() return values:
	 *
	 * -1: error
	 *  0: child process context
	 * >0: (child pid) parent context.
	 *
	 * Since child process has its own data(global_data) and stack(local_data)
	 * segments, when it update the variables, a new page entry is allocated
	 * (copy-on-write) and variables values of parent are not changed.
	 */
	switch (cpid = fork()) {
	case -1:
		ERROR("fork()\n");
		return -1;

	case 0:
		DEBUG("[child] altering local_data...\n");
		local_data++;
		//
		DEBUG("[child] altering global_data...\n");
		global_data++;
		//
		DEBUG("[child] PPID=%d; PID=%d; global_data=%d; local_data=%d\n",
				getppid(), getpid(), global_data, local_data);
		//
		DEBUG("[child] exiting...\n");
		exit(0);
	default:
		DEBUG("[parent] sleeping...\n");
		sleep(5);
		//
		DEBUG("[parent] PPID=%d; PID=%d; global_data=%d; local_data=%d\n",
				getppid(), getpid(), global_data, local_data);
		//
		if (wait(NULL) == -1) {
			ERROR("wait()\n");
			return -1;
		}
		//
		DEBUG("[parent] child has finished...\n");
		break;
	}

	return 0;
}
