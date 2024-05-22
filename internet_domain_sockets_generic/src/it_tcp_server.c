/**
 * TCP server implementation using generic function in utils.
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
	socklen_t addrlen;
	ssize_t recv_bytes;
	char host[NI_MAXHOST];
	char serv[NI_MAXSERV];
	char _buf[BUFFER_SIZE];
	int sock_fd, client_fd;
	struct sockaddr_in sa_client;

	/*********************************************************
	 * overwrite SIGPIPE signal
	 *
	 * If server try writing to a client socket, where client
	 * has already closed the socket, a SIGPIPE signal will be
	 * generated
	 ********************************************************/
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		ERROR("signal() failed: %s!\n", strerror(errno));
		goto finish;
	}

	/*********************************************************
	 * create, bind and listen socket
	 ********************************************************/
	sock_fd = generic_listen(SERVER_PORT, 10, SOCK_STREAM, AF_INET);
	if (sock_fd == -1) {
		ERROR("generic_listen() failed!\n");
		goto finish;
	}

	/*********************************************************
	 * accept connections and read data from clients
	 ********************************************************/
	while (1) {
		//
		addrlen = sizeof(struct sockaddr_storage);
		client_fd = accept(sock_fd, (struct sockaddr*)&sa_client, &addrlen);
		if (client_fd == -1) {
			ERROR("acccept() failed: %s!\n", strerror(errno));
			continue;
		}

		//
		if (sock2name((struct sockaddr*)&sa_client, addrlen, host, serv)) {
			ERROR("sock2name() failed!\n");
			continue;
		}

		//
		DEBUG("(client) HOST: %s\n", host);
		DEBUG("(client) SERV: %s\n", serv);

		// client connection started
		while (1) {
			recv_bytes = recv(client_fd, _buf, BUFFER_SIZE, 0);
			if (recv_bytes == -1) {
				ERROR("recv() failed: %s!\n", strerror(errno));
				goto next_connection;
			}

			// client closed the connection
			if (recv_bytes == 0) {
				DEBUG("Connection closed!\n");
				goto next_connection;
			}

			// print data from peer
			DEBUG("Data received: [%.*s]!\n", (int)recv_bytes, _buf);
		}
next_connection:
		close(client_fd);
	}

finish:
	return 0;
}
