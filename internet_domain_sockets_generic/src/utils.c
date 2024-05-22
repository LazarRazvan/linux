/**
 * Utils implementation with generic function that can be used to create a
 * client-server application by using getaddrinfo() system call, without using
 * harcoded values for IPs and ports.
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
#include <arpa/inet.h>
#include <sys/socket.h>

#include "debug.h"
#include "utils.h"


/*============================================================================*/

/**
 * Function to debug sockaddr data structure.
 */
static void
__hint_dump(struct addrinfo *hint)
{
	void *ip_addr;
	unsigned short port;
	struct sockaddr_in *ip4;
	struct sockaddr_in6 *ip6;
	char ip_buf[INET6_ADDRSTRLEN];

	//
	switch (hint->ai_family) {
	case AF_INET:
		ip4 = (struct sockaddr_in *)hint->ai_addr;
		ip_addr = (void *)&ip4->sin_addr;
		break;
	case AF_INET6:
		ip6 = (struct sockaddr_in6 *)hint->ai_addr;
		ip_addr = (void *)&ip6->sin6_addr;
		break;
	default:
		ERROR("Unknown family type!");
		break;
	}

	//
	if (!inet_ntop(hint->ai_family, ip_addr, ip_buf, INET6_ADDRSTRLEN)) {
		ERROR("inet_ntop() failed!\n");
		return;
	}

	//
	switch (hint->ai_family) {
	case AF_INET:
		port = ip4->sin_port;
		break;
	case AF_INET6:
		port = ip6->sin6_port;
		break;
	default:
		break;
	}

	//
	DEBUG("IP  : %s\n", ip_buf);
	DEBUG("PORT: %d\n", ntohs(port));
}


/*============================================================================*/

/**
 * Function to convert socket address into human readable format by performing
 * name resolution.
 *
 * @saddr    : Socket address structure.
 * @saddrlen : Socket address structure length.
 * @host     : User specified buffer to store host. (size must be NI_MAXHOST)
 * @serv     : User specified buffer to store serv. (size must be NI_MAXHOST)
 *
 * Return 0 on success and -1 on error.
 */
int
sock2name(struct sockaddr *saddr, size_t saddrlen, char *host, char *serv)
{

	/*********************************************************
	 * perform getnameinfo()
	 *
	 * Note that port is specified as number inside socket
	 * address, so we are safe to skip name resolution.
	 ********************************************************/
	return getnameinfo(saddr, saddrlen, host, NI_MAXHOST, serv, NI_MAXSERV,
					NI_NUMERICSERV);
}

/**
 * Generic function to establish a connection.
 *
 * @host     : Hostname value as string. (may be NULL).
 * @serv     : Service value as string. (may be NULL).
 * @sock_type: Socket type (stream or datagram).
 * @fam_type : Family type (ipv4, ipv6 or unspec).
 *
 * Return a socket id on success that has performed connect and is ready to
 * send/receive data or -1 on error.
 */
int
generic_connect(char *host, char *serv, int sock_type, int fam_type)
{
	int status, sock_fd = -1;
	struct addrinfo hints, *res, *it;

	//
	sock_fd = -1;

	/*********************************************************
	 * arguments validation
	 ********************************************************/
	switch (sock_type) {
	case SOCK_STREAM:
	case SOCK_DGRAM:
		break;
	default:
		ERROR("Invalid socket type!\n");
		goto finish;
	}

	//
	switch (fam_type) {
	case AF_INET:
	case AF_INET6:
	case AF_UNSPEC:
		break;
	default:
		ERROR("Invalid family type!\n");
		goto finish;
	}

	/*********************************************************
	 * configure selection criteria
	 *
	 * Note that ai_addrlen, ai_canonname, ai_addr and ai_next
	 * should not be set here because are only considered in
	 * results
	 ********************************************************/
	memset(&hints, 0, sizeof(hints));
	hints.ai_family		= fam_type;
	hints.ai_socktype	= sock_type;
	hints.ai_protocol	= 0;
	hints.ai_flags		= 0;
	hints.ai_canonname	= NULL;
	hints.ai_addr		= NULL;
	hints.ai_next		= NULL;

	/*********************************************************
	 * get results
	 *
	 * 1) Perform name resolution if the host is set which may
	 * be time consuming (AI_NUMERICHOST must be set in flags
	 * to disable and consider host as an ip address).
	 *
	 * 2) Perform name resolution if the service is set which
	 * may be time consuming (AI_NUMERICSERV must be set in
	 * flags to disable and treat service as a numeric port).
	 *
	 * 3) If the host is not set, the returned ip addresses
	 * will be set to loopback (INADDR_LOOPBACK or
	 * INA6DDR_LOOBACK_INIT), that are suitable for a active
	 * connection (connect()/sendto()).
	 *
	 * 4) If the service is not set, the returned port will
	 * be 0.
	 ********************************************************/
	status = getaddrinfo(host, serv, &hints, &res);
	if (status) {
		ERROR("getaddrinfo() error: %s!\n", gai_strerror(status));
		goto finish;
	}

	/*********************************************************
	 * try to establish a connection
	 ********************************************************/
	for (it = res; it; it = it->ai_next) {
#if DEBUG_ENABLE
		DEBUG("Trying connection...!\n");
		__hint_dump(it);
#endif

		// open socket
		sock_fd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
		if (!sock_fd) {
			ERROR("socket() failed: %s!\n", strerror(errno));
			continue;
		}

		// connect socket (ip + port)
		if (connect(sock_fd, it->ai_addr, it->ai_addrlen)) {
			ERROR("connect() failed: %s!\n", strerror(errno));
			close(sock_fd);
			sock_fd = -1;
			continue;
		}

		// success
		break;
	}

//free_results:
	freeaddrinfo(res);

finish:
	return sock_fd;
}

/**
 * Generic function to bind a socket.
 *
 * @serv     : Service value as string. (may be NULL).
 * @sock_type: Socket type (stream or datagram).
 * @fam_type : Family type (ipv4, ipv6 or unspec).
 *
 * Return a socket id on success that has performed bind if a suitable socket
 * is found and is ready or -1 on error.
 */
int
generic_bind(char *serv, int sock_type, int fam_type)
{
	struct addrinfo hints, *res, *it;
	int status, sock_fd = -1, option;

	//
	sock_fd = -1;

	/*********************************************************
	 * arguments validation
	 ********************************************************/
	switch (sock_type) {
	case SOCK_STREAM:
	case SOCK_DGRAM:
		break;
	default:
		ERROR("Invalid socket type!\n");
		goto finish;
	}

	//
	switch (fam_type) {
	case AF_INET:
	case AF_INET6:
	case AF_UNSPEC:
		break;
	default:
		ERROR("Invalid family type!\n");
		goto finish;
	}

	/*********************************************************
	 * configure selection criteria
	 *
	 * Note that ai_addrlen, ai_canonname, ai_addr and ai_next
	 * should not be set here because are only considered in
	 * results
	 ********************************************************/
	memset(&hints, 0, sizeof(hints));
	hints.ai_family		= fam_type;
	hints.ai_socktype	= sock_type;
	hints.ai_protocol	= 0;
	hints.ai_flags		= AI_PASSIVE;
	hints.ai_canonname	= NULL;
	hints.ai_addr		= NULL;
	hints.ai_next		= NULL;

	/*********************************************************
	 * get results
	 *
	 * 1) host is NULL in this case because we are looking for
	 * a passive connection. Since AI_PASSIVE flag is set, the
	 * suitable socket address will contain a wildcard address
	 * (INADDR_ANY or IN6ADDR_ANY_INIT). Binding on this ip
	 * addresses ensures caputre of any incoming packet.
	 *
	 * 2) Perform name resolution if the service is set which
	 * may be time consuming (AI_NUMERICSERV must be set in
	 * flags to disable and treat service as a numeric port).
	 *
	 * 3) Note that since host is NULL by default, service
	 * must be set, otherwise getaddrinfo() will return an
	 * error.
	 ********************************************************/
	status = getaddrinfo(NULL, serv, &hints, &res);
	if (status) {
		ERROR("getaddrinfo() error: %s!\n", gai_strerror(status));
		goto finish;
	}

	/*********************************************************
	 * try to perfrom the bind
	 ********************************************************/
	option = 1;

	//
	for (it = res; it; it = it->ai_next) {
#if DEBUG_ENABLE
		DEBUG("Trying bind...!\n");
		__hint_dump(it);
#endif

		// open socket
		sock_fd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
		if (!sock_fd) {
			ERROR("socket() failed: %s!\n", strerror(errno));
			continue;
		}

#if ENABLE_SOCKET_REUSE
		// prevent address already in use for rapid restarts (TIME_WAIT)
		if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &option,
						sizeof(option)))
		{
			ERROR("setsockopt() failed: %s!\n", strerror(errno));
			close(sock_fd);
			sock_fd = -1;
			continue;
		}
#endif

		// connect socket (ip + port)
		if (bind(sock_fd, it->ai_addr, it->ai_addrlen)) {
			ERROR("bind() failed: %s!\n", strerror(errno));
			close(sock_fd);
			sock_fd = -1;
			continue;
		}

		// success
		break;
	}

//free_results:
	freeaddrinfo(res);

finish:
	return sock_fd;
}

/**
 * Generic function to listen on a socket.
 *
 * @serv     : Service value as string. (may be NULL).
 * @backlog  : Connection queue size.
 * @sock_type: Socket type (stream or datagram).
 * @fam_type : Family type (ipv4, ipv6 or unspec).
 *
 * Return a socket id on success that has performed bind if a suitable socket
 * is found and is ready or -1 on error.
 */
int
generic_listen(char *serv, int backlog, int sock_type, int fam_type)
{
	int sock_fd = -1;

	/*********************************************************
	 * make sure bind succeed and return us the socket fd
	 ********************************************************/
	sock_fd = generic_bind(serv, sock_type, fam_type);
	if (sock_fd == -1) {
		ERROR("generic_bind() failed!\n");
		goto finish;
	}

	/*********************************************************
	 * perform listen on socket.
	 *
	 * backlog represent the queue size maintained by the
	 * kernel representing the number of connection that can
	 * be queued before accept() is called.
	 ********************************************************/
	if (listen(sock_fd, backlog)) {
		ERROR("listen() failed: %s!\n", strerror(errno));
		sock_fd = -1;
		goto finish;
	}

finish:
	return sock_fd;
}
