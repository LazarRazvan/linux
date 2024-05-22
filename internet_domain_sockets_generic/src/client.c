/**
 * TCP client implementation using generic function in utils.
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
	ssize_t read_bytes, send_bytes;

	/*********************************************************
	 * connect to server listening socket
	 *
	 * Use localhost since running locally.
	 ********************************************************/
	sock_fd = generic_connect("localhost", SERVER_PORT, SOCK_STREAM, AF_INET);
	if (sock_fd == -1) {
		ERROR("generic_connect() failed!\n");
		goto finish;
	}

	/*********************************************************
	 * read data from stdin and send to server
	 ********************************************************/
	while (1) {
		// read
		read_bytes = read(STDIN_FILENO, _buf, BUFFER_SIZE);
		if (read_bytes == -1) {
			ERROR("read() failed: %s!\n", strerror(errno));
			goto sock_close;
		}

		// send
		send_bytes = send(sock_fd, _buf, read_bytes, 0);
		if (send_bytes == -1) {
			ERROR("Send failed: %s!\n", strerror(errno));
			goto sock_close;
		}

		if (send_bytes != read_bytes) {
			ERROR("Partial send to server...!\n");
		}

		DEBUG("Send data [%.*s]\n", (int)send_bytes, _buf);
	}

sock_close:
	close(sock_fd);
finish:
	return 0;
}
