/**
 * Create a new process using vfork().
 * Copyright (C) 2023 Lazar Razvan.
 *
 * Expose the mechanism of creating a new process using vfork() system call.
 *
 * When using vfork() (mostly used when exec() call is used next) the new
 * process shares the same memory with the parent process.
 *
 * Note that parent process is suspende until child process either call exit()
 * or exec().
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
	 * Create a child process. vfork() return values:
	 *
	 * -1: error
	 *  0: child process context
	 * >0: (child pid) parent context.
	 *
	 * Since child process shares the same memory with the paret, including
	 * data(global_data) and stack(local_data) segments, when it update the
	 * variables, parent will have find the updated values.
	 */
	switch (cpid = vfork()) {
	case -1:
		ERROR("vfork()\n");
		return -1;

	case 0:
		DEBUG("[child] sleeping...\n");
		sleep(5);
		//
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
