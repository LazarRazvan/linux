/**
 * Client/Server example for unix domain datagram sockets unix socketpair.
 *
 * Copyright (C) 2024 Lazar Razvan.
 *
 * Datagram sockets are datagram oriented, meaning that messages boundaries
 * are preserved. For unix domain sockets, AF_UNIX(PF_UNIX) must be used
 * and sockaddr_un data structure.
 *
 * Logic:
 *
 * 1) PARENT: socketpair()
 * 		Create a datagram sockets pair by parent.
 *
 * 2) PARENT: fork()
 * 		Create child (client) process that inherit and use on end of sockets.
 *
 * 3) PARENT: bind()
 * 		Bind the socket to an to an entry in the filesystem.
 *
 * 4) PARENT: recvfrom()
 * 		Receive datagram from client.
 *
 * 5) CHILD: sendto()
 * 		Send datagram data to server.
 *
 * Note that although there is no order ensured for datagram sockets, since we
 * use unix domain sockets, the connection is handled by kernel so all
 * datagrams will be received in order.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include <sys/un.h>
#include <sys/wait.h>
#include <sys/socket.h>

#include "debug.h"


/*============================================================================*/

// Child socket fd index
#define CHILD_SOCK_IDX				0

// Parent socket fd index
#define PARENT_SOCK_IDX				1

// Stream server socket
#define SERVER_SOCK_PATH			"/tmp/datagram_server_pair_sock"

// Stream listen backlog (connection that are not blocked)
#define BUFFER_SIZE					128


/*============================================================================*/

#define DATAGRAMS_NO				10

static const char *datagram_msg[DATAGRAMS_NO] = {

	[0] = "datagram0",
	[1] = "datagram1",
	[2] = "datagram2",
	[3] = "datagram3",
	[4] = "datagram4",
	[5] = "datagram5",
	[6] = "datagram6",
	[7] = "datagram7",
	[8] = "datagram8",
	[9] = "datagram9"
};


/*============================================================================*/

int main(int argc, char *argv[])
{
	pid_t pid;
	int sock_fds[2];
	char _buf[BUFFER_SIZE];
	struct sockaddr_un sa_server;
	ssize_t send_bytes, recv_bytes;

	/*********************************************************
	 * create datagram socket pairs
	 ********************************************************/
	if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sock_fds) == -1) {
		ERROR("Socket pairs creation failed: %s!\n", strerror(errno));
		goto error;
	}

	/*********************************************************
	 * create child process (client)
	 ********************************************************/
	errno = 0;
	switch (pid = fork()) {
	case -1:
		ERROR("Fork failed: %s!\n", strerror(errno));
		goto socket_pair_close;

	case 0:
		/*********************************************************
		 * client pid
		 ********************************************************/
		DEBUG("[CLIENT] pid: %d\n", getpid());
		sleep(2);

		/*********************************************************
		 * client send
		 ********************************************************/
		if (strlen(SERVER_SOCK_PATH) >= sizeof(sa_server.sun_path)) {
			ERROR("[CLIENT] Socket connect path exceed buffer size!");
			exit(-1);
		}

		memset(&sa_server, 0, sizeof(struct sockaddr_un));
		sa_server.sun_family = AF_UNIX;
		memcpy(sa_server.sun_path, SERVER_SOCK_PATH, strlen(SERVER_SOCK_PATH));

		for (int i = 0; i < DATAGRAMS_NO; i++) {
			// send data to server
			errno = 0;
			send_bytes = sendto(sock_fds[CHILD_SOCK_IDX], datagram_msg[i],
								strlen(datagram_msg[i]), 0,
								(struct sockaddr *)&sa_server,
								sizeof(struct sockaddr_un));
			if (send_bytes == -1) {
				ERROR("[CLIENT] Send failed: %s!\n", strerror(errno));
				exit(-1);
			}

			//
			assert(send_bytes == strlen(datagram_msg[i]));

			//
			DEBUG("[CLIENT] Send [%s]\n", datagram_msg[i]);
		}

		DEBUG("[CLIENT] Exiting!\n");
		exit(0);
	default:
		/*********************************************************
		 * server pid
		 ********************************************************/
		DEBUG("[SERVER] pid: %d\n", getpid());

		/*********************************************************
		 * server bind
		 ********************************************************/
		if (strlen(SERVER_SOCK_PATH) >= sizeof(sa_server.sun_path)) {
			ERROR("Socket path exceed buffer size!");
			goto wait_child;
		}

		memset(&sa_server, 0, sizeof(struct sockaddr_un));
		sa_server.sun_family = AF_UNIX;
		memcpy(sa_server.sun_path, SERVER_SOCK_PATH, strlen(SERVER_SOCK_PATH));

		//
		if (remove(sa_server.sun_path) == -1 && errno != ENOENT) {
			ERROR("File %s deletion failed: %s\n", sa_server.sun_path,
					strerror(errno));
			goto wait_child;
		}

		//
		errno = 0;
		if (bind(sock_fds[PARENT_SOCK_IDX], (struct sockaddr *)&sa_server,
											sizeof(struct sockaddr_un)) == -1) {
			ERROR("Socket bind failed: %s!\n", strerror(errno));
			goto wait_child;
		}

		/*********************************************************
		 * server recv
		 ********************************************************/
		DEBUG("[SERVER] Waiting for data...\n");
		for (int i = 0; i < DATAGRAMS_NO; i++) {
			// recv data from client
			errno = 0;
			recv_bytes = recvfrom(sock_fds[PARENT_SOCK_IDX], _buf, BUFFER_SIZE,
									0, NULL, 0);
			if (recv_bytes == -1) {
				ERROR("Recv from client failed: %s!\n", strerror(errno));
				continue;
			}

			// empty datagram
			if (recv_bytes == 0) {
				ERROR("Recv empty datagram from client!\n");
				continue;
			}

			DEBUG("[SERVER] Recv [%.*s]!\n", (int)recv_bytes, _buf);
		}

		/*********************************************************
		 * server waiting from child
		 ********************************************************/
		break;
	}

	return 0;

wait_child:
	DEBUG("[SERVER] Waiting for child...\n");
	errno = 0;
	if (wait(NULL) == -1)
		ERROR("Wait failed: %s!\n", strerror(errno));
socket_pair_close:
	close(sock_fds[0]);
	close(sock_fds[1]);
error:
	return -1;
}
