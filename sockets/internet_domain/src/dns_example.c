/**
 * Simple example of DNS (Domain Name System) work flow using getaddrinfo()
 * system call.
 *
 * Copyright (C) 2024 Lazar Razvan.
 *
 * Recursive vs Iterative resolution
 *
 * 1) Recursive
 *
 * The client sends the entire request to DNS server to be resolved.
 *
 * 2) Iterative
 *
 * The client iterative sends nodes of the request to find out the next DNS
 * server until the entire request is resolved.
 *
 * 	For request: www.google.com
 * 		1st request: com
 *		2nd request: google.com
 *		3rd request: www.google.com
 *
 * getaddrinfo()
 * 	First try to recursive resolve the request using local DNS server. If the
 * 	request is not resolved, it uses an iterative approach.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <netdb.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "debug.h"


/*============================================================================*/

// DNS request
#define DNS_REQUEST					"www.kernel.org"


/*============================================================================*/

int main(int argc, char *argv[])
{
	int status;
	void *ip_addr;
	struct addrinfo hints;
	struct sockaddr_in *ip4;
	struct sockaddr_in6 *ip6;
	struct addrinfo *res, *it;
	char ip_buf[INET6_ADDRSTRLEN];

	/*********************************************************
	 * configure criteria for selection
	 ********************************************************/
	memset(&hints, 0, sizeof(hints));
	hints.ai_family		= AF_UNSPEC;	// allow IPv4 or IPv6
	hints.ai_socktype	= SOCK_DGRAM;	// datagram socket
	hints.ai_protocol	= 0;			// any protocol
	hints.ai_flags		= 0;
	hints.ai_canonname	= NULL;
	hints.ai_addr		= NULL;
	hints.ai_next		= NULL;

	/*********************************************************
	 * get results
	 ********************************************************/
	status = getaddrinfo(DNS_REQUEST, NULL, &hints, &res);
	if (status) {
		ERROR("getaddrinfo error: %s\n", gai_strerror(status));
		goto error;
	}

	/*********************************************************
	 * dump results
	 ********************************************************/
	printf("DNS results for host '%s':\n", DNS_REQUEST);

	//
	for (it = res; it; it = it->ai_next) {
		//
		switch (it->ai_family) {
		case AF_INET:
			ip4 = (struct sockaddr_in *)it->ai_addr;
			ip_addr = (void *)&ip4->sin_addr;
			break;
		case AF_INET6:
			ip6 = (struct sockaddr_in6 *)it->ai_addr;
			ip_addr = (void *)&ip6->sin6_addr;
			break;
		default:
			ERROR("Unknown family type!");
			goto error;
		}

		//
		if (!inet_ntop(it->ai_family, ip_addr, ip_buf, INET6_ADDRSTRLEN))
			ERROR("inet_ntop error\n");

		//
		printf("  %s\n", ip_buf);
	}

	//
	freeaddrinfo(res);

	return 0;

error:
	return -1;
}
