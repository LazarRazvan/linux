#ifndef DATAGRAM_COMMON_H
#define DATAGRAM_COMMON_H

/*============================================================================*/

// Force using of abstract sockets (no entries in filesystem)
#define ENABLE_ABSTRACT_SOCKET		0

// Force client to bind to an address (set to 0 for disable)
#define ENABLE_CLIENT_BIND			1

// Client bind address format (if enable)
#if ENABLE_CLIENT_BIND
#	define CLIENT_SOCK_FMT			"/tmp/datagram_client_sock_%d"
#endif


/*============================================================================*/

// Stream server socket
#define SERVER_SOCK_PATH			"/tmp/datagram_server_sock"

// Stream listen backlog (connection that are not blocked)
#define SERVER_SOCK_BACKLOG			10

// Stream listen backlog (connection that are not blocked)
#define BUFFER_SIZE					128


#endif	// DATAGRAM_COMMON_H

