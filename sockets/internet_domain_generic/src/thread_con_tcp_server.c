
/**
 * TCP concurent server implementation using generic function in utils for
 * echo server.
 *
 * Mechanism implemented using a new thread each time a new connection is
 * accepted.
 *
 * Each thread perform pthread_detach() so that when it terminates, its
 * resources will be automatically released back to the system without the need
 * of a join.
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
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "debug.h"
#include "utils.h"
#include "common.h"


/*============================================================================*/

/**
 * Thread data structure.
 */
typedef struct thread_data_s {

	int					sfd;			// client socket descriptor
	struct sockaddr 	sa_client;		// client socket address
	socklen_t			len;			// client address length

} thread_data_t;


/*============================================================================*/


/**
 * Client connection handler.
 */
static void *
__connection_handler(void *arg)
{
	pthread_t tid;
	ssize_t recv_bytes;
	thread_data_t *data;
	char host[NI_MAXHOST];
	char serv[NI_MAXSERV];
	char _buf[BUFFER_SIZE];

	//
	tid = pthread_self();
	data = (thread_data_t *)arg;

	//
	pthread_detach(tid);

	//
	if (sock2name(&data->sa_client, data->len, host, serv)) {
		ERROR("[%lu] sock2name() failed!\n", tid);
		goto finish;
	}

	//
	while (1) {
		recv_bytes = recv(data->sfd, _buf, BUFFER_SIZE, 0);
		if (recv_bytes == -1) {
			ERROR("[%lu] recv() failed: %s!\n", tid, strerror(errno));
			goto finish;
		}

		//
		if (recv_bytes == 0) {
			DEBUG("[%lu] Connection closed!\n", tid);
			goto finish;
		}

		//
		DEBUG("[%lu][%s: %s] Recv: [%.*s]!\n", tid, host, serv, (int)recv_bytes,
											_buf);

		//
		if (send(data->sfd, _buf, recv_bytes, 0) != recv_bytes) {
			ERROR("[%lu] send() failed: %s!\n", tid, strerror(errno));
			goto finish;
		}
	}

finish:
	close(data->sfd);
	free(data);
	return NULL;
}


/*============================================================================*/

int main(int argc, char *argv[])
{
	pthread_t tid;
	int listen_fd;
	thread_data_t *data;

	/*********************************************************
	 * create listening socket
	 ********************************************************/
	listen_fd = generic_listen(SERVER_PORT, 10, SOCK_STREAM, AF_INET);
	if (listen_fd == -1) {
		ERROR("generic_listen() failed!\n");
		goto finish;
	}

	/*********************************************************
	 * accept connections and create new processes
	 ********************************************************/
	while (1) {
		//
		data = malloc(sizeof(thread_data_t));
		if (!data) {
			ERROR("malloc() failed!\n");
			continue;
		}

		//
		data->len = sizeof(struct sockaddr_storage);
		data->sfd = accept(listen_fd, &data->sa_client, &data->len);
		if (data->sfd == -1) {
			ERROR("acccept() failed: %s!\n", strerror(errno));
			continue;
		}

		//
		if (pthread_create(&tid, NULL, __connection_handler, data) != 0) {
			ERROR("pthread_create() failed: %s!\n", strerror(errno));
			free(data);
			continue;
		}
	}

finish:
	return 0;
}
