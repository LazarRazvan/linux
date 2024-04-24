/**
 * Client for unix domain stream sockets application, reading information
 * from stdin and pass to server.
 *
 * Copyright (C) 2024 Lazar Razvan.
 *
 * Stream sockets are byte oriented, meaning that there are no boundaries
 * for the messages. For unix domain sockets, AF_UNIX(PF_UNIX) must be used
 * and sockaddr_un data structure.
 *
 * System calls to be used:
 * - send(): Send data to stream socket.
 * - recv(): Read data from stream socket.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include <sys/un.h>
#include <sys/socket.h>

#include "debug.h"
#include "stream_common.h"


/*============================================================================*/

int main(int argc, char *argv[])
{
	int sock_fd;
	socklen_t len;
	char _buf[BUFFER_SIZE];
	ssize_t send_bytes, read_bytes;
	struct sockaddr_un sa_client, sa_server;

	/*********************************************************
	 * create the unix domain, stream socket
	 ********************************************************/
	sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock_fd == -1) {
		ERROR("Socket creation failed: %s!\n", strerror(errno));
		goto error;
	}

	//
	errno = 0;
	len = sizeof(struct sockaddr_un);
	if (getsockname(sock_fd, (struct sockaddr *)&sa_client, &len) == -1) {
		ERROR("Client info failed: %s!\n", strerror(errno));
		goto sock_close;
	}

	/*********************************************************
	 * connect to server
	 ********************************************************/
	if (strlen(SERVER_SOCK_PATH) >= sizeof(sa_server.sun_path)) {
		ERROR("Socket path exceed buffer size!");
		goto sock_close;
	}

	memset(&sa_server, 0, sizeof(struct sockaddr_un));
	sa_server.sun_family = AF_UNIX;
	memcpy(sa_server.sun_path, SERVER_SOCK_PATH, strlen(SERVER_SOCK_PATH));

	errno = 0;
	if (connect(sock_fd, (struct sockaddr *)&sa_server,
										sizeof(struct sockaddr_un)) == -1) {
		ERROR("Socket connect failed: %s!\n", strerror(errno));
		goto sock_close;
	}

	/*********************************************************
	 * accept new connections (one at a time)
	 *
	 * When the peer close the socket, the recv() system call
	 * will receive end-of-file, knowing that the client has
	 * close the connection.
	 *
	 * When server close the socket, the send() system call
	 * will receive a SIGPIPE.
	 ********************************************************/
	while (1) {
		// read data from stdin
		errno = 0;
		read_bytes = read(STDIN_FILENO, _buf, BUFFER_SIZE);
		if (read_bytes == -1) {
			ERROR("Stdin read failed: %s!\n", strerror(errno));
			goto sock_close;
		}

		// send data to server
		send_bytes = send(sock_fd, _buf, read_bytes, 0);
		if (send_bytes == -1) {
			ERROR("Send failed: %s!\n", strerror(errno));
			goto sock_close;
		}

		//
		assert(send_bytes == read_bytes);

		//
		DEBUG("Client %s send data [%.*s]\n", sa_client.sun_path,
				(int)read_bytes, _buf);
	}

	return 0;

sock_close:
	close(sock_fd);
error:
	return -1;
}
