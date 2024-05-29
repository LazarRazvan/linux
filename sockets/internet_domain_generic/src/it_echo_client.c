/**
 * UDP iterative client implementation using generic function in utils for
 * echo server.
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
#include <arpa/inet.h>
#include <sys/socket.h>

#include "debug.h"
#include "utils.h"
#include "common.h"


/*============================================================================*/

int main(int argc, char *argv[])
{
	int sock_fd;
	char _buf[BUFFER_SIZE];
	ssize_t recv_bytes, read_bytes, send_bytes;

	/*********************************************************
	 * create client socket, connected to server
	 *
	 * Note that using generic_connect, we may use send() and
	 * recv() functions (instead of sendto() and recvfrom())
	 * breacuse the socket will remeber the server address.
	 * Also, the datagrams received from someone else than the
	 * server will not be received.
	 ********************************************************/
	sock_fd = generic_connect("localhost", SERVER_PORT, SOCK_DGRAM, AF_INET);
	if (sock_fd == -1) {
		ERROR("socket() failed: %s!\n", strerror(errno));
		goto finish;
	}

	/*********************************************************
	 * read data from stdin, send to server and read it back
	 ********************************************************/
	while (1) {
		//
		printf("Enter client data:\n");

		//
		read_bytes = read(STDIN_FILENO, _buf, BUFFER_SIZE);
		if (read_bytes == -1) {
			ERROR("read() failed: %s!\n", strerror(errno));
			continue;
		}

		//
		send_bytes = send(sock_fd, _buf, read_bytes, 0);
		if (send_bytes == -1) {
			ERROR("send() failed: %s!\n", strerror(errno));
			continue;
		}

		//
		if (send_bytes != read_bytes) {
			ERROR("Partial send to server...!\n");
		}

		recv_bytes = recv(sock_fd, _buf, BUFFER_SIZE, 0);
		if (recv_bytes == -1) {
			ERROR("recv() failed: %s!\n", strerror(errno));
			continue;
		}

		//
		if (send_bytes != recv_bytes) {
			ERROR("Partial read from server...!\n");
		}
	}

finish:
	return 0;
}

