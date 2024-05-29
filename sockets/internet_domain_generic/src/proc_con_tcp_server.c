/**
 * TCP concurent  server implementation using generic function in utils for
 * echo server.
 *
 * Mechanism implemented using a new process (fork()) each time a new
 * connection is accepted.
 *
 * Copyright (C) 2024 Lazar Razvan.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <netdb.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "debug.h"
#include "utils.h"
#include "common.h"


/*============================================================================*/

/**
 * Signal handler to wait for zombie child processes.
 *
 * When client close the connection with server, the associated process perform
 * exit() system call triggering SIGCHLD signal to parent. If the parent does
 * not properly wait for the process, it will become zombie.
 */
static void
__closed_connection_handler(int signal_no)
{
	int errno_bak;

	// save errno
	errno_bak = errno;

	// loop to handle all zombie processes (note that call will not block
	// because we use WHOHANG option
	while (waitpid(-1, NULL, WNOHANG) != -1);

	// restore errno
	errno = errno_bak;
}

/**
 * Client connection handler.
 */
static void
__connection_handler(int sfd, struct sockaddr *sa_client, socklen_t len)
{
	pid_t pid;
	ssize_t recv_bytes;
	char host[NI_MAXHOST];
	char serv[NI_MAXSERV];
	char _buf[BUFFER_SIZE];

	//
	pid = getpid();

	//
	if (sock2name(sa_client, len, host, serv)) {
		ERROR("[%d] sock2name() failed!\n", pid);
		goto finish;
	}

	//
	while (1) {
		recv_bytes = recv(sfd, _buf, BUFFER_SIZE, 0);
		if (recv_bytes == -1) {
			ERROR("[%d] recv() failed: %s!\n", pid, strerror(errno));
			goto finish;
		}

		//
		if (recv_bytes == 0) {
			DEBUG("[%d] Connection closed!\n", pid);
			goto finish;
		}

		//
		DEBUG("[%d][%s: %s] Recv: [%.*s]!\n", pid, host, serv, (int)recv_bytes,
											_buf);

		//
		if (send(sfd, _buf, recv_bytes, 0) != recv_bytes) {
			ERROR("[%d] send() failed: %s!\n", pid, strerror(errno));
			goto finish;
		}
	}

finish:
	close(sfd);
	exit(1);
}


/*============================================================================*/

int main(int argc, char *argv[])
{
	socklen_t addrlen;
	struct sigaction sa;
	int listen_fd, client_fd;
	struct sockaddr sa_client;

	/*********************************************************
	 * overwrite SIGCHLD signal
	 *
	 * When connection is closed with client, the process will
	 * become zombie, so we need to wait for it.
	 ********************************************************/
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = __closed_connection_handler;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		ERROR("sigaction() failed: %s!\n", strerror(errno));
		goto finish;
	}

	/*********************************************************
	 * create listening socket
	 ********************************************************/
	listen_fd = generic_listen(SERVER_PORT, 10, SOCK_STREAM, AF_INET);
	if (listen_fd == -1) {
		ERROR("generic_listen() failed!\n");
		goto finish;
	}

	/*********************************************************
	 * accept connections and create new processes
	 ********************************************************/
	while (1) {
		//
		addrlen = sizeof(struct sockaddr_storage);
		client_fd = accept(listen_fd, &sa_client, &addrlen);
		if (client_fd == -1) {
			ERROR("acccept() failed: %s!\n", strerror(errno));
			continue;
		}

		switch (fork()) {
		case -1:
			ERROR("fork() failed: %s!\n", strerror(errno));
			break;
		case 0:
			// child
			close(listen_fd);	// do not need the listening socket
			__connection_handler(client_fd, &sa_client, addrlen);
			exit(1);
		default:
			// parent
			close(client_fd);	// do not need the client socket
			break;
		}
	}

finish:
	return 0;
}
