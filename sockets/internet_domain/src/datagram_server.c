/**
 * Server for internet domain datagram sockets application, reading information
 * from client and dumping to stdout.
 *
 * Copyright (C) 2024 Lazar Razvan.
 *
 * Datagram sockets are datagram oriented, meaning that messages boundaries
 * are preserved. For internet domain sockets, AF_INET/AF_INET6 must be used
 * and sockaddr_in/sockaddr_in6 data structure.
 *
 * Note that unlike unix datagram sockets, were receive/send buffers management
 * is performed by kernel, causing the blocking of the read/write system call
 * when the buffers are full, for internet domain sockets, a full buffer of the
 * receiver will lead to packet drop.
 *
 * Logic:
 *
 * 1) socket()
 * 		Create a datagram socket of internet domain.
 *
 * 2) bind()
 * 		Bind the socket to a pair of ip and port number.
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
#include <arpa/inet.h>
#include <sys/socket.h>

#include "debug.h"
#include "datagram_common.h"


/*============================================================================*/

int main(int argc, char *argv[])
{
	int sock_fd;
	socklen_t len;
	ssize_t recv_bytes;
	char _buf[BUFFER_SIZE];
	char addr_client[INET_ADDRSTRLEN];
	struct sockaddr_in sa_client, sa_server;

	/*********************************************************
	 * create the unix domain, datagram socket
	 ********************************************************/
	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_fd == -1) {
		ERROR("Socket creation failed: %s!\n", strerror(errno));
		goto error;
	}

	/*********************************************************
	 * bind socket to an address (make sure path not exceed buffer)
	 *
	 * On success, the kernel will create a new entry in the
	 * filesystem for the associated socket address.
	 ********************************************************/
	memset(&sa_server, 0, sizeof(struct sockaddr_in));
	sa_server.sin_family = AF_INET;
	sa_server.sin_port = htons(SERVER_PORT);
	if (inet_pton(AF_INET, SERVER_ADDR, &sa_server.sin_addr) <= 0) {
		ERROR("Unable to convert ip address: %s\n", SERVER_ADDR);
	}

	//
	errno = 0;
	if (bind(sock_fd, (struct sockaddr *)&sa_server,
										sizeof(struct sockaddr_in)) == -1) {
		ERROR("Socket bind failed: %s!\n", strerror(errno));
		goto socket_close;
	}

	/*********************************************************
	 * read datagrams from clients.
	 *
	 * recvfrom() system call will block, waiting for data
	 * from the client.
	 ********************************************************/
	while (1) {
		// read data from client
		errno = 0;
		len = sizeof(struct sockaddr_in);
		recv_bytes = recvfrom(sock_fd, _buf, BUFFER_SIZE, 0,
							(struct sockaddr *)&sa_client, &len);
		if (recv_bytes == -1) {
			ERROR("Recv from client failed: %s!\n", strerror(errno));
			continue;
		}

		// empty datagram
		if (recv_bytes == 0) {
			ERROR("Recv empty datagram from client!\n");
			continue;
		}

		if (!inet_ntop(AF_INET, &sa_client.sin_addr, addr_client,
															INET_ADDRSTRLEN)) {
			ERROR("Unable to convert client ip address!\n");
		}

		DEBUG("Recv from client {%s:%u}: [%.*s]!\n", addr_client,
				ntohs(sa_client.sin_port), (int)recv_bytes, _buf);
	}

	return 0;

socket_close:
	close(sock_fd);
error:
	return -1;
}
