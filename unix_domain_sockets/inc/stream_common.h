#ifndef STREAM_COMMON_H
#define STREAM_COMMON_H

/*============================================================================*/

// Force using of abstract sockets (no entries in filesystem)
#define ENABLE_ABSTRACT_SOCKET		1

// Force client to bind to an address (set to 0 for disable)
#define ENABLE_CLIENT_BIND			1

// Client bind address format (if enable)
#if ENABLE_CLIENT_BIND
#	define CLIENT_SOCK_FMT			"/tmp/stream_client_sock_%d"
#endif


/*============================================================================*/

// Stream server socket
#define SERVER_SOCK_PATH			"/tmp/stream_server_sock"

// Stream listen backlog (connection that are not blocked)
#define SERVER_SOCK_BACKLOG			10

// Stream listen backlog (connection that are not blocked)
#define BUFFER_SIZE					128


#endif	// STREAM_COMMON_H

