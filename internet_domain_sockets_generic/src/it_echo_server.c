/**
 * UDP iterative server implementation using generic function in utils for
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
	socklen_t addrlen;
	ssize_t recv_bytes;
	char host[NI_MAXHOST];
	char serv[NI_MAXSERV];
	char _buf[BUFFER_SIZE];
	struct sockaddr_storage sa_client;

	/*********************************************************
	 * bind socket to an address for incoming connections
	 ********************************************************/
	sock_fd = generic_bind(SERVER_PORT, SOCK_DGRAM, AF_INET);
	if (sock_fd == -1) {
		ERROR("generic_bind() failed!\n");
		goto finish;
	}

	/*********************************************************
	 * read data from clients and send it back
	 ********************************************************/
	while (1) {
		recv_bytes = recvfrom(sock_fd, _buf, BUFFER_SIZE, 0,
							(struct sockaddr *)&sa_client, &addrlen);
		if (recv_bytes == -1) {
			ERROR("recvfrom() failed: %s!\n", strerror(errno));
			continue;
		}

		//
		if (sock2name((struct sockaddr*)&sa_client, addrlen, host, serv)) {
			ERROR("sock2name() failed!\n");
			continue;
		}

		//
		DEBUG("[%s: %s] Recv: [%.*s]!\n", host, serv, (int)recv_bytes, _buf);

		//
		if (sendto(sock_fd, _buf, recv_bytes, 0,(struct sockaddr *)&sa_client,
					addrlen) != recv_bytes) {
			ERROR("sendto() failed: %s!\n", strerror(errno));
			continue;
		}
	}

finish:
	return 0;
}
