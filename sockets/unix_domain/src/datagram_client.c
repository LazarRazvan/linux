/**
 * Client for unix domain datagram sockets application, reading information
 * from stdin and pass to server.
 *
 * Copyright (C) 2024 Lazar Razvan.
 *
 * Datagram sockets are datagram oriented, meaning that messages boundaries
 * are preserved. For unix domain sockets, AF_UNIX(PF_UNIX) must be used
 * and sockaddr_un data structure.
 *
 * Logic:
 *
 * 1) socket()
 * 		Create a datagram socket of unix domain.
 *
 * 2) bind() (optional if ENABLE_CLIENT_BIND)
 * 		Bind the socket to an abstract address (if ENABLE_ABSTRACT_SOCKET) or
 * 	to an entry in the filesystem.
 *
 * 3) sendto()
 * 		Send datagram data to server.
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
#include "datagram_common.h"


/*============================================================================*/

int main(int argc, char *argv[])
{
	int sock_fd;
	char _buf[BUFFER_SIZE];
	ssize_t send_bytes, read_bytes;
	struct sockaddr_un sa_client, sa_server;

	/*********************************************************
	 * create the unix domain, datagram socket
	 ********************************************************/
	sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sock_fd == -1) {
		ERROR("Socket creation failed: %s!\n", strerror(errno));
		goto error;
	}

#if ENABLE_CLIENT_BIND

#if ENABLE_ABSTRACT_SOCKET
	/*********************************************************
	 * create an abstract socket.
	 *
	 * This is ensured by writing the sun_path buffer after
	 * the first byte that MUST be '\0'. In this case, the
	 * will be no entry in the filesystem for the associated
	 * socket address.
	 ********************************************************/
	if (strlen(SERVER_SOCK_PATH) + 6 >= sizeof(sa_client.sun_path)) {
		ERROR("Socket bind path exceed buffer size!");
		goto sock_close;
	}

	//
	memset(&sa_client, 0, sizeof(struct sockaddr_un));
	sa_client.sun_family = AF_UNIX;
	if (sprintf(&sa_client.sun_path[1], CLIENT_SOCK_FMT, getpid()) < 0) {
		ERROR("Socket bind path creation failed!");
		goto sock_close;
	}
#else
	/*********************************************************
	 * bind socket to an address (make sure path not exceed buffer)
	 *
	 * On success, the kernel will create a new entry in the
	 * filesystem for the associated socket address.
	 ********************************************************/
	if (strlen(SERVER_SOCK_PATH) + 5 >= sizeof(sa_client.sun_path)) {
		ERROR("Socket bind path exceed buffer size!");
		goto sock_close;
	}

	//
	memset(&sa_client, 0, sizeof(struct sockaddr_un));
	sa_client.sun_family = AF_UNIX;
	if (sprintf(sa_client.sun_path, CLIENT_SOCK_FMT, getpid()) < 0) {
		ERROR("Socket bind path creation failed!");
		goto sock_close;
	}

	//
	errno = 0;
	if (remove(sa_client.sun_path) == -1 && errno != ENOENT) {
		ERROR("File %s deletion failed: %s\n", sa_client.sun_path,
				strerror(errno));
		goto sock_close;
	}
#endif	// ENABLE_ABSTRACT_SOCKET

	//
	errno = 0;
	if (bind(sock_fd, (struct sockaddr *)&sa_client,
										sizeof(struct sockaddr_un)) == -1) {
		ERROR("Socket bind failed: %s!\n", strerror(errno));
		goto sock_close;
	}
#endif	// ENABLE_CLIENT_BIND


	/*********************************************************
	 * configure server address
	 ********************************************************/
#if ENABLE_ABSTRACT_SOCKET
	if (strlen(SERVER_SOCK_PATH) + 1 >= sizeof(sa_server.sun_path)) {
		ERROR("Socket connect path exceed buffer size!");
		goto sock_close;
	}

	memset(&sa_server, 0, sizeof(struct sockaddr_un));
	sa_server.sun_family = AF_UNIX;
	memcpy(&sa_server.sun_path[1], SERVER_SOCK_PATH, strlen(SERVER_SOCK_PATH));
#else
	if (strlen(SERVER_SOCK_PATH) >= sizeof(sa_server.sun_path)) {
		ERROR("Socket connect path exceed buffer size!");
		goto sock_close;
	}

	memset(&sa_server, 0, sizeof(struct sockaddr_un));
	sa_server.sun_family = AF_UNIX;
	memcpy(sa_server.sun_path, SERVER_SOCK_PATH, strlen(SERVER_SOCK_PATH));
#endif	// ENABLE_ABSTRACT_SOCKET

	/*********************************************************
	 * send datagrams to server
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
		errno = 0;
		send_bytes = sendto(sock_fd, _buf, read_bytes, 0,
							(struct sockaddr *)&sa_server,
							sizeof(struct sockaddr_un));
		if (send_bytes == -1) {
			ERROR("Send failed: %s!\n", strerror(errno));
			goto sock_close;
		}

		//
		assert(send_bytes == read_bytes);

		//
		DEBUG("Send data [%.*s]\n", (int)read_bytes, _buf);
	}

	return 0;

sock_close:
	close(sock_fd);
error:
	return -1;
}
