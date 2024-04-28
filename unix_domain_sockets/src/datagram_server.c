/**
 * Server for unix domain datagram sockets application, reading information
 * from client and dumping to stdout.
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
 * 2) bind()
 * 		Bind the socket to an abstract address (if ENABLE_ABSTRACT_SOCKET) or
 * 	to an entry in the filesystem.
 *
 * 3) recvfrom()
 * 		Recevie datagram data from client.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/un.h>
#include <sys/socket.h>

#include "debug.h"
#include "datagram_common.h"


/*============================================================================*/

int main(int argc, char *argv[])
{
	socklen_t len;
	socklen_t optlen;
	ssize_t recv_bytes;
	int sock_size, sock_fd;
	char _buf[BUFFER_SIZE];
	struct sockaddr_un sa_client, sa_server;

	/*********************************************************
	 * create the unix domain, datagram socket
	 ********************************************************/
	sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sock_fd == -1) {
		ERROR("Socket creation failed: %s!\n", strerror(errno));
		goto error;
	}

	/*********************************************************
	 * DEBUG: print max/min datagram size
	 ********************************************************/
	// max datagram size
	errno = 0;
	optlen = sizeof(sock_size);
	if (getsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, &sock_size, &optlen) < 0) {
		ERROR("Socket send size failed: %s!\n", strerror(errno));
		goto socket_close;
	}

	DEBUG("Datagram max send: %d\n", sock_size);

	// min datagram size
	errno = 0;
	optlen = sizeof(sock_size);
	if (getsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, &sock_size, &optlen) < 0) {
		ERROR("Socket send size failed: %s!\n", strerror(errno));
		goto socket_close;
	}

	DEBUG("Datagram max recv: %d\n", sock_size);

#if ENABLE_ABSTRACT_SOCKET
	/*********************************************************
	 * create an abstract socket.
	 *
	 * This is ensured by writing the sun_path buffer after
	 * the first byte that MUST be '\0'. In this case, the
	 * will be no entry in the filesystem for the associated
	 * socket address.
	 ********************************************************/
	if (strlen(SERVER_SOCK_PATH) + 1 >= sizeof(sa_server.sun_path)) {
		ERROR("Socket path exceed buffer size!");
		goto socket_close;
	}

	memset(&sa_server, 0, sizeof(struct sockaddr_un));
	sa_server.sun_family = AF_UNIX;
	memcpy(&sa_server.sun_path[1], SERVER_SOCK_PATH, strlen(SERVER_SOCK_PATH));
#else
	/*********************************************************
	 * bind socket to an address (make sure path not exceed buffer)
	 *
	 * On success, the kernel will create a new entry in the
	 * filesystem for the associated socket address.
	 ********************************************************/
	if (strlen(SERVER_SOCK_PATH) >= sizeof(sa_server.sun_path)) {
		ERROR("Socket path exceed buffer size!");
		goto socket_close;
	}

	memset(&sa_server, 0, sizeof(struct sockaddr_un));
	sa_server.sun_family = AF_UNIX;
	memcpy(sa_server.sun_path, SERVER_SOCK_PATH, strlen(SERVER_SOCK_PATH));

	//
	if (remove(sa_server.sun_path) == -1 && errno != ENOENT) {
		ERROR("File %s deletion failed: %s\n", sa_server.sun_path,
				strerror(errno));
		goto socket_close;
	}
#endif	// ENABLE_ABSTRACT_SOCKET

	//
	errno = 0;
	if (bind(sock_fd, (struct sockaddr *)&sa_server,
										sizeof(struct sockaddr_un)) == -1) {
		ERROR("Socket bind failed: %s!\n", strerror(errno));
		goto socket_close;
	}

	/*********************************************************
	 * read datagrams from clients.
	 *
	 * recvfrom() system call will block, waiting for data
	 * from the client.
	 *
	 * Note that unlinke stream sockets, datagrams unix domain
	 * sockets allow zero-length datagram.
	 ********************************************************/
	while (1) {
		// read data from client
		errno = 0;
		len = sizeof(struct sockaddr_un);
		recv_bytes = recvfrom(sock_fd, _buf, BUFFER_SIZE, 0,
							(struct sockaddr *)&sa_client, &len);
		if (recv_bytes == -1) {
			ERROR("Recv from client failed: %s!\n", strerror(errno));
			continue;
		}

		// empty datagram
		if (recv_bytes == 0) {
			ERROR("Recv empty datagram from client %s!\n", sa_client.sun_path);
			continue;
		}

		DEBUG("Recv from client %s: [%.*s]!\n", sa_client.sun_path,
				(int)recv_bytes, _buf);

	}

	return 0;

socket_close:
	close(sock_fd);
error:
	return -1;
}
