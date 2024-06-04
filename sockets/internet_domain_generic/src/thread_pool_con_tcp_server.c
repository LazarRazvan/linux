
/**
 * TCP concurent server implementation using generic function in utils for
 * echo server.
 *
 * Mechanism is implemented using a thread pool, to speed up the process and
 * get rid of the overhead of creating a new thread for each new connection.
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
 * Maximum number of concuret connections.
 */
#define MAX_CON_CONNECTIONS				4


/*============================================================================*/


/**
 * Connection data structure.
 */
typedef struct conn_data_s {

	struct sockaddr 	sa_client;		// client socket address
	socklen_t			len;			// client address length
	int					sfd;			// client socket descriptor

} conn_data_t;


/*============================================================================*/


/**
 * Thread pool data structure.
 */
typedef struct thread_pool_data_s {

	pthread_t 			tids[MAX_CON_CONNECTIONS];	// threads ids
	conn_data_t 		data[MAX_CON_CONNECTIONS];	// threads data

	pthread_mutex_t		lock;						// pool lock
	pthread_cond_t		cond_not_full;				// pool full condition
	pthread_cond_t		cond_not_empty;				// pool empty condition

	int					head;						// pool head
	int					tail;						// pool tail
	int					size;						// pool size
	int					capacity;					// pool capacity

} thread_pool_data_t;


/*============================================================================*/

/**
 * Thread pool API.
 */

// forward declaration
static void *__connection_handler(void *arg);

// thread pool global memory
thread_pool_data_t _pool;

static inline int
__thread_pool_is_full(void)
{
	return (_pool.size == _pool.capacity);
}

static inline int
__thread_pool_is_empty(void)
{
	return (_pool.size == 0);
}

static int
__thread_pool_init(void)
{

	//
	_pool.size = 0;
	_pool.head = 0;
	_pool.tail = 0;
	_pool.capacity = MAX_CON_CONNECTIONS;

	//
	if (pthread_mutex_init(&_pool.lock, NULL)) {
		ERROR("pthread_mutex_init() failed: %s!\n", strerror(errno));
		goto error;
	}

	//
	if (pthread_cond_init(&_pool.cond_not_full, NULL)) {
		ERROR("pthread_cond_init() failed: %s!\n", strerror(errno));
		goto mutex_free;
	}

	//
	if (pthread_cond_init(&_pool.cond_not_empty, NULL)) {
		ERROR("pthread_cond_init() failed: %s!\n", strerror(errno));
		goto cond_not_full_free;
	}

	//
	for (int i = 0; i < MAX_CON_CONNECTIONS; i++) {
		if (pthread_create(&_pool.tids[i], NULL, __connection_handler, NULL)) {
			ERROR("pthread_create() failed: %s!\n", strerror(errno));
			goto cond_not_empty_free;
		}
	}

// success
	return 0;

cond_not_empty_free:
	pthread_cond_destroy(&_pool.cond_not_empty);
cond_not_full_free:
	pthread_cond_destroy(&_pool.cond_not_full);
mutex_free:
	pthread_mutex_destroy(&_pool.lock);
error:
	return -1;
}

static void
__thread_pool_enqueue(int sfd, struct sockaddr *sa_client, socklen_t len)
{
	conn_data_t *conn;

	//
	pthread_mutex_lock(&_pool.lock);

	while (__thread_pool_is_full())
		pthread_cond_wait(&_pool.cond_not_full, &_pool.lock);

	//
	conn = &_pool.data[_pool.tail];
	conn->sfd		= sfd;
	conn->len		= len;
	conn->sa_client	= *sa_client;

	//
	_pool.tail = (_pool.tail + 1) % _pool.capacity;
	_pool.size++;

	//
	pthread_cond_signal(&_pool.cond_not_empty);

	//
	pthread_mutex_unlock(&_pool.lock);
}

static conn_data_t *
__thread_pool_dequeue(void)
{
	conn_data_t *conn;

	//
	pthread_mutex_lock(&_pool.lock);

	while (__thread_pool_is_empty())
		pthread_cond_wait(&_pool.cond_not_empty, &_pool.lock);

	//
	conn = &_pool.data[_pool.head];

	//
	_pool.head = (_pool.head + 1) % _pool.capacity;
	_pool.size--;

	//
	pthread_cond_signal(&_pool.cond_not_full);

	//
	pthread_mutex_unlock(&_pool.lock);

	return conn;
}

static void
__thread_pool_destroy(void)
{

	//
	pthread_mutex_destroy(&_pool.lock);
	pthread_cond_destroy(&_pool.cond_not_full);
	pthread_cond_destroy(&_pool.cond_not_empty);

	//
	for (int i = 0; i < MAX_CON_CONNECTIONS; i++)
		pthread_join(_pool.tids[i], NULL);
}


/*============================================================================*/

/**
 * Connction handler.
 */
static void *
__connection_handler(void *arg)
{
	pthread_t tid;
	conn_data_t conn;
	ssize_t recv_bytes;
	char host[NI_MAXHOST];
	char serv[NI_MAXSERV];
	char _buf[BUFFER_SIZE];

	while (1) {
		//
		tid = pthread_self();
		conn = *(conn_data_t *)__thread_pool_dequeue();

		//
		if (sock2name(&conn.sa_client, conn.len, host, serv)) {
			ERROR("[%lu] sock2name() failed!\n", tid);
			close(conn.sfd);
			continue;
		}

		//
		while (1) {
			recv_bytes = recv(conn.sfd, _buf, BUFFER_SIZE, 0);
			if (recv_bytes == -1) {
				ERROR("[%lu] recv() failed: %s!\n", tid, strerror(errno));
				break;
			}

			//
			if (recv_bytes == 0) {
				DEBUG("[%lu] Connection closed!\n", tid);
				break;
			}

			//
			DEBUG("[%lu][%s: %s] Recv: [%.*s]!\n", tid, host, serv,
												(int)recv_bytes, _buf);

			//
			if (send(conn.sfd, _buf, recv_bytes, 0) != recv_bytes) {
				ERROR("[%lu] send() failed: %s!\n", tid, strerror(errno));
				break;
			}
		}

// next_connection:
		close(conn.sfd);
	}

	return NULL;
}


/*============================================================================*/


int main(int argc, char *argv[])
{
	int sfd;
	int listen_fd;
	socklen_t len;
	struct sockaddr sa_client;

	/*********************************************************
	 * thread pool initialization
	 ********************************************************/
	if (__thread_pool_init()) {
		ERROR("thread_pool_init() failed!\n");
		goto finish;
	}

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
		len = sizeof(struct sockaddr_storage);
		sfd = accept(listen_fd, &sa_client, &len);
		if (sfd == -1) {
			ERROR("acccept() failed: %s!\n", strerror(errno));
			continue;
		}

		//
		__thread_pool_enqueue(sfd, &sa_client, len);
	}

	/*********************************************************
	 * thread pool destroy
	 ********************************************************/
	__thread_pool_destroy();

finish:
	return 0;
}
