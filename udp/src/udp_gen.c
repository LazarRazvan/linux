/**
 * UDP Packet Generator.
 * Copyright (C) 2024 Lazar Razvan.
 *
 * Usage:
 * ./run/udp_gen [-src_ip <ip>] [-src_port <port>] [-dst_ip <ip>]
 * 					[-dst_port <port>] [-payload_size <size>]
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include "debug.h"
#include "udp_gen.h"


/*============================================================================*/

//
// Application command line arguments
//
#define CMD_SRC_IP					"-src_ip"
#define CMD_DST_IP					"-dst_ip"
#define CMD_SRC_PORT				"-src_port"
#define CMD_DST_PORT				"-dst_port"
#define CMD_PLD_SIZE				"-payload_size"

//
// Application default config
//
#define DEFAULT_SRC_IP				"127.0.0.1"
#define DEFAULT_DST_IP				"127.0.0.1"
#define DEFAULT_SRC_PORT			5000
#define DEFAULT_DST_PORT			5000
#define DEFAULT_UDP_PAYLOAD_SIZE	32


/*============================================================================*/

//
// Application default values
//
char *src_ip		= DEFAULT_SRC_IP;
char *dst_ip		= DEFAULT_DST_IP;
short src_port		= DEFAULT_SRC_PORT;
short dst_port		= DEFAULT_DST_PORT;
int payload_size	= DEFAULT_UDP_PAYLOAD_SIZE;


/*============================================================================*/

/**
 * Checksum function.
 *
 * @b	: Buffer.
 * @len	: Buffer length.
 *
 * Return the 16-bit ones' complement.
 */
unsigned short
__checksum(void *b, int len)
{
	unsigned short *buf = b;
	unsigned int sum = 0;
	unsigned short result;

	for (sum = 0; len > 1; len -= 2)
		sum += *buf++;
	if (len == 1)
		sum += *(unsigned char *)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;

	return result;
}


/*============================================================================*/

/**
 * Packet dump.
 */
static void
__packet_dump(const unsigned char *packet, int length) {
	int i, j;

	printf("Dumping packet packet (%d bytes):\n", length);
	printf("Offse(h)  Hexadecimal Representation                       ASCII Representation\n");
	printf("--------  -----------------------------------------------  --------------------\n");

	for (i = 0; i < length; i += 16) {
		printf("%08x  ", i);  // Offset in hexadecimal

		// Print hex values
		for (j = 0; j < 16; j++) {
			if (i + j < length)
				printf("%02x ", packet[i + j]);
			else
				printf("   ");
		}

		printf(" ");
		// Print ASCII values
		for (j = 0; j < 16; j++) {
			if (i + j < length) {
				unsigned char c = packet[i + j];
				if (c >= 32 && c <= 126)  // Readable ASCII range
					printf("%c", c);
				else
					printf(".");
			}
		}
		printf("\n");
	}
	printf("\n");
}


/*============================================================================*/

/**
 * Fill in the ipv4 header of the packet.
 *
 * @ip_hdr	: Ipv4 header.
 * @ip_src	: Source ip address.
 * @ip_dst	: Destination ip address.
 * @pk_size	: Total packet size.
 */
static void
__set_ip_hdr(struct ip *ip_hdr, char *ip_src, char *ip_dst, int pk_size)
{
	ip_hdr->ip_v			= IPVERSION;
	ip_hdr->ip_hl			= 5;			// 20 bytes (no options)
	ip_hdr->ip_tos			= 0;
	ip_hdr->ip_len			= htons(pk_size);
	ip_hdr->ip_ttl			= 255;
	ip_hdr->ip_off			= 0;
	ip_hdr->ip_id			= htons(12345);
	ip_hdr->ip_p			= IPPROTO_UDP;
	ip_hdr->ip_src.s_addr	= inet_addr(ip_src);
	ip_hdr->ip_dst.s_addr	= inet_addr(ip_dst);
	ip_hdr->ip_sum 			= __checksum((void *)ip_hdr, pk_size);
}


/*============================================================================*/

/**
 * Fill in the udp header of the packet.
 *
 * @udp_hdr	: Udp header.
 * @port_src: Source port.
 * @port_dst: Destination port.
 * @pk_size	: Total packet size.
 */
static void
__set_udp_hdr(struct udphdr *udp_hdr, short port_src, short port_dst,
			int payload_size)
{
	DEBUG("port_src: %d\n", port_src);
	DEBUG("port_dst: %d\n", port_dst);
	DEBUG("size: %d\n", sizeof(struct udphdr) + payload_size);

	udp_hdr->uh_sport	= htons(port_src);
	udp_hdr->uh_dport	= htons(port_dst);
	udp_hdr->uh_ulen	= htons(sizeof(struct udphdr) + payload_size);
	udp_hdr->uh_sum 	= __checksum((void *)udp_hdr, ntohs(udp_hdr->uh_ulen));
}


/*============================================================================*/

/**
 * Fill in the udp payload.
 *
 * @udp_payload_addr: Udp payload address.
 * @udp_payload_size: Udp payload size.
 */
static void
__set_udp_payload(void *udp_payload_addr, int udp_payload_size)
{
	char *c;

	for (int i = 0; i < udp_payload_size; i++) {
		c = (char *)(udp_payload_addr + i);
		*c = 'a' + i % ('z' - 'a');
	}
}


/*============================================================================*/

/**
 * Fill in the udp payload.
 *
 * @src_ip		: Source ip.
 * @dst_ip		: Destination ip.
 * @src_port	: Source port.
 * @dst_port	: Destination port.
 * @payload_size: Payload size.
 */
static void
__send_packet(void)
{
	void *pk_addr;
	void *payload;
	struct ip *ip;
	struct udphdr *udp;
	struct sockaddr_in sa;
	int bytes_sent, sockfd, pk_size;

	// create raw socket (IPPROTO_UDP, OS should deal with Ethernet)
	//sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
	sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
	if (sockfd == -1) {
		ERROR("Unable to open socket: %s!\n", strerror(errno));
		goto finish;
	}

	// compute packet size
	pk_size = PK_SIZE(payload_size);

	// create packet memory
	pk_addr = malloc(pk_size);
	if (!pk_addr) {
		ERROR("Unable to create packet memory!\n");
		goto close_socket;
	}

	// map structures to packet memory
	ip = (struct ip *)IP_HDR_OFFSET(pk_addr);
	udp = (struct udphdr *)UDP_HDR_OFFSET(pk_addr);
	payload = UDP_PAYLOAD_OFFSET(pk_addr);

	// debug print packet
	DEBUG("pk_addr      = %p\n", pk_addr);
	DEBUG("pk_size      = %d\n", pk_size);
	DEBUG("ip_addr      = %p\n", ip);
	DEBUG("udp_addr     = %p\n", udp);
	DEBUG("payload_addr = %p\n", payload);

	// complete destination addr
	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(dst_port);
	sa.sin_addr.s_addr = inet_addr(dst_ip);

	// set udp payload
	__set_udp_payload(payload, payload_size);

	// set udp header
	__set_udp_hdr(udp, src_port, dst_port, payload_size);

	// set ip header
	__set_ip_hdr(ip, src_ip, dst_ip, pk_size);

#if DEBUG_ENABLE
	__packet_dump(pk_addr, pk_size);
#endif

	bytes_sent = sendto(sockfd, pk_addr, pk_size, 0, (struct sockaddr *)&sa,
						sizeof(sa));
	//
	DEBUG("Sent %d bytes!\n", bytes_sent);

	//
	if (bytes_sent == -1)
		ERROR("Unable to send the packet: %s!\n", strerror(errno));

//free_packet:
	free(pk_addr);
close_socket:
	close(sockfd);
finish:
	return;
}


/*============================================================================*/

int main(int argc, char *argv[])
{

	// parse command line arguments
    for (int i = 1; i < argc; i++) {
		// source ip
        if (strcmp(argv[i], CMD_SRC_IP) == 0) {
            src_ip = argv[++i];
		}

		// destination ip
        if (strcmp(argv[i], CMD_DST_IP) == 0) {
            dst_ip = argv[++i];
		}

		// source port
        if (strcmp(argv[i], CMD_SRC_PORT) == 0) {
            src_port = atoi(argv[++i]);
		}

		// destination port
        if (strcmp(argv[i], CMD_DST_PORT) == 0) {
            src_port = atoi(argv[++i]);
		}

		// payload size
        if (strcmp(argv[i], CMD_PLD_SIZE) == 0) {
            src_port = atoi(argv[++i]);
		}
    }

	// debug print arguments
	DEBUG("Source ip        = %s\n", src_ip);
	DEBUG("Destination ip   = %s\n", dst_ip);
	DEBUG("Source port      = %d\n", src_port);
	DEBUG("Destination port = %d\n", dst_port);
	DEBUG("Payload size     = %d\n", payload_size);

	// send packet
	__send_packet();

	return 0;
}
