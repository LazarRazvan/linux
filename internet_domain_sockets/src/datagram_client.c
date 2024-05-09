/**
 * Client for internet domain datagram sockets application, reading information
 * from stdin and pass to server.
 *
 * Copyright (C) 2024 Lazar Razvan.
 *
 * Datagram sockets are datagram oriented, meaning that messages boundaries
 * are preserved. For internet domain sockets, AF_INET/AF_INET6 must be used
 * and sockaddr_in/sockaddr_in6 data structure.
 *
 * Logic:
 *
 * 1) socket()
 * 		Create a datagram socket of unix domain.
 *
 * 2) bind()
 * 		Bind the socket to a pair of ip and port number.
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
#include <arpa/inet.h>
#include <sys/socket.h>

#include "debug.h"
#include "datagram_common.h"


/*============================================================================*/

int main(int argc, char *argv[])
{
	int sock_fd;
	char _buf[BUFFER_SIZE];
	ssize_t send_bytes, read_bytes;
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
	 ********************************************************/
	memset(&sa_client, 0, sizeof(struct sockaddr_in));
	sa_client.sin_family = AF_INET;
	sa_client.sin_port = htons(CLIENT_PORT);
	if (inet_pton(AF_INET, CLIENT_ADDR, &sa_client.sin_addr) <= 0) {
		ERROR("Unable to convert ip address: %s\n", CLIENT_ADDR);
	}

	//
	errno = 0;
	if (bind(sock_fd, (struct sockaddr *)&sa_client,
										sizeof(struct sockaddr_in)) == -1) {
		ERROR("Socket bind failed: %s!\n", strerror(errno));
		goto sock_close;
	}

	/*********************************************************
	 * configure server address
	 ********************************************************/
	memset(&sa_server, 0, sizeof(struct sockaddr_in));
	sa_server.sin_family = AF_INET;
	sa_server.sin_port = htons(SERVER_PORT);
	if (inet_pton(AF_INET, SERVER_ADDR, &sa_server.sin_addr) <= 0) {
		ERROR("Unable to convert ip address: %s\n", SERVER_ADDR);
		goto sock_close;
	}

	/*********************************************************
	 * send datagrams to server
	 *
	 * If server receive buffer is full, the packets will be
	 * dropped.
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
							sizeof(struct sockaddr_in));
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
