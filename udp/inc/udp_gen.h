#ifndef UDP_GEN_H
#define UDP_GEN_H

/*============================================================================*/

// packet size
#define PK_SIZE(payload_size)	\
		((sizeof(struct ip)) + (sizeof(struct udphdr)) + (payload_size))

// ip hdr offset
#define IP_HDR_OFFSET(pk_addr)	\
		((void *)(pk_addr))

// udp hdr offset
#define UDP_HDR_OFFSET(pk_addr)	\
		((void *)(IP_HDR_OFFSET(pk_addr) + sizeof(struct ip)))

// udp payload offset
#define UDP_PAYLOAD_OFFSET(pk_addr)	\
		((void *)(UDP_HDR_OFFSET(pk_addr) + sizeof(struct ip)))


#endif	// UDP_GEN_H

