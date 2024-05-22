#ifndef UTILS_H
#define UTILS_H


/*============================================================================*/

// Enable socket address reuse (useful when restaring)
#define ENABLE_SOCKET_REUSE					1


/*============================================================================*/

// Generic connect (client)
int generic_connect(char *host, char *serv, int sock_type, int fam_type);

// Generic bind (server)
int generic_bind(char *serv, int sock_type, int fam_type);

// Generic listen (server)
int generic_listen(char *serv, int backlog, int sock_type, int fam_type);

// Perform name resolution for a socket entry
int sock2name(struct sockaddr *saddr, size_t saddrlen, char *host, char *serv);


#endif	// UTILS_H

