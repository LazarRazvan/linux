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
 * Logic:
 *
 * 1) socket()
 * 		Create a stream socket of unix domain.
 *
 * 2) bind()
 * 		Bind the socket to an abstract address (if ENABLE_ABSTRACT_SOCKET) or
 * 	to an entry in the filesystem.
 *
 * 3) listen()
 * 		Waiting for incoming connections.
 *
 * 4) accept()
 * 		Accept an incoming connection.
 *
 * 5) recv()
 * 		Recevie stream data from client.
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
	struct sockaddr_un sa_client, sa_server;

	/*********************************************************
	 * create the unix domain, stream socket
	 ********************************************************/
	listen_sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (listen_sock == -1) {
		ERROR("Socket creation failed: %s!\n", strerror(errno));
		goto error;
	}

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
		goto listen_socket_close;
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
		goto listen_socket_close;
	}

	memset(&sa_server, 0, sizeof(struct sockaddr_un));
	sa_server.sun_family = AF_UNIX;
	memcpy(sa_server.sun_path, SERVER_SOCK_PATH, strlen(SERVER_SOCK_PATH));

	//
	if (remove(sa_server.sun_path) == -1 && errno != ENOENT) {
		ERROR("File %s deletion failed: %s\n", sa_server.sun_path,
				strerror(errno));
		goto listen_socket_close;
	}
#endif	// ENABLE_ABSTRACT_SOCKET

	//
	errno = 0;
	if (bind(listen_sock, (struct sockaddr *)&sa_server,
										sizeof(struct sockaddr_un)) == -1) {
		ERROR("Socket bind failed: %s!\n", strerror(errno));
		goto listen_socket_close;
	}

	/*********************************************************
	 * listen for new connections
	 *
	 * Depending on the SERVER_SOCK_BACKLOG, the connect()
	 * system call performed by client may block.
	 ********************************************************/
	errno = 0;
	if (listen(listen_sock, SERVER_SOCK_BACKLOG) == -1) {
		ERROR("Socket listen failed: %s!\n", strerror(errno));
		goto bind_entry_remove;
	}

	/*********************************************************
	 * accept new connections (one at a time)
	 *
	 * Blocks until a new connection is received.
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
			ERROR("Socket accept failed: %s!\n", strerror(errno));
			continue;
		}

		// get peer info
		errno = 0;
		len = sizeof(struct sockaddr_un);

		if (getpeername(peer_sock, (struct sockaddr *)&sa_client, &len) == -1) {
			ERROR("Socket peer failed: %s!\n", strerror(errno));
			goto next_connection;
		}

		// get peer data (block until data is received)
		while (1) {
			errno = 0;
			recv_bytes = recv(peer_sock, _buf, BUFFER_SIZE, 0);
			if (recv_bytes == -1) {
				ERROR("Recv from peer %s failed: %s!\n", sa_client.sun_path,
						strerror(errno));
				goto next_connection;
			}

			// peer closed the connection
			if (recv_bytes == 0) {
				ERROR("Peer %s closed the connection!\n", sa_client.sun_path);
				goto next_connection;
			}

			// print data from peer
			DEBUG("Recv from peer %s: [%.*s]!\n", sa_client.sun_path,
					(int)recv_bytes, _buf);
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
