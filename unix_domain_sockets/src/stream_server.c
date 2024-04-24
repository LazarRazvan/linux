/**
 * Server for unix domain stream sockets application, reading information
 * from client and dumping to stdout.
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
#include <errno.h>

#include <sys/un.h>
#include <sys/socket.h>

#include "debug.h"
#include "stream_common.h"


/*============================================================================*/

int main(int argc, char *argv[])
{
	socklen_t len;
	ssize_t recv_bytes;
	char _buf[BUFFER_SIZE];
	int listen_sock, peer_sock;
	struct sockaddr_un sa_peer, sa_unix;

	/*********************************************************
	 * create the unix domain, stream socket
	 ********************************************************/
	listen_sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (listen_sock == -1) {
		ERROR("Socket creation failed: %s!\n", strerror(errno));
		goto error;
	}

	/*********************************************************
	 * bind socket to an address (make sure path not exceed buffer)
	 *
	 * On success, the kernel will create a new entry in the
	 * filesystem for the associated socket address.
	 ********************************************************/
	if (strlen(SERVER_SOCK_PATH) >= sizeof(sa_unix.sun_path)) {
		ERROR("Socket path exceed buffer size!");
		goto listen_socket_close;
	}

	memset(&sa_unix, 0, sizeof(struct sockaddr_un));
	sa_unix.sun_family = AF_UNIX;
	memcpy(sa_unix.sun_path, SERVER_SOCK_PATH, strlen(SERVER_SOCK_PATH));

	errno = 0;
	if (bind(listen_sock, (struct sockaddr *)&sa_unix,
										sizeof(struct sockaddr_un)) == -1) {
		ERROR("Socket bind failed: %s!\n", strerror(errno));
		goto listen_socket_close;
	}

	/*********************************************************
	 * listen for new connections
	 *
	 * Blocks until a new connection is received.
	 ********************************************************/
	errno = 0;
	if (listen(listen_sock, SERVER_SOCK_BACKLOG) == -1) {
		ERROR("Socket listen failed: %s!\n", strerror(errno));
		goto bind_entry_remove;
	}

	/*********************************************************
	 * accept new connections (one at a time)
	 *
	 * When the peer close the socket, the recv() system call
	 * will receive end-of-file, knowing that the client has
	 * close the connection.
	 *
	 * At first recv() system call, server will block, waiting
	 * for data to read.
	 ********************************************************/
	while (1) {
		errno = 0;
		peer_sock = accept(listen_sock, NULL, NULL);
		if (peer_sock == -1) {
			ERROR("Accept sock failed: %s!\n", strerror(errno));
			continue;
		}

		// get peer info
		errno = 0;
		len = sizeof(struct sockaddr_un);

		if (getpeername(peer_sock, (struct sockaddr *)&sa_peer, &len) == -1) {
			ERROR("Peer info failed: %s!\n", strerror(errno));
			goto next_connection;
		}

		// get peer data (block until data is received)
		while (1) {
			errno = 0;
			recv_bytes = recv(peer_sock, _buf, BUFFER_SIZE, 0);
			if (recv_bytes == -1) {
				ERROR("Fail to read data from peer %s: %s!\n",
						sa_peer.sun_path, strerror(errno));
				goto next_connection;
			}

			// peer closed the connection
			if (recv_bytes == 0) {
				ERROR("Peer %s closed the connection!\n", sa_peer.sun_path);
				goto next_connection;
			}

			// print data from peer
			DEBUG("Peer %s: [%.*s]!\n", sa_peer.sun_path, (int)recv_bytes,
					_buf);
		}

next_connection:
		close(peer_sock);
	}

	return 0;

bind_entry_remove:
	remove(SERVER_SOCK_PATH);
listen_socket_close:
	close(listen_sock);
error:
	return -1;
}
