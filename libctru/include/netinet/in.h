#pragma once

#include <stdint.h>
#include <sys/socket.h>

#define INADDR_LOOPBACK  0x7f000001
#define INADDR_ANY       0x00000000
#define INADDR_BROADCAST 0xFFFFFFFF
#define INADDR_NONE      0xFFFFFFFF

#define INET_ADDRSTRLEN  16

/*
 * Protocols (See RFC 1700 and the IANA)
 */
#define IPPROTO_IP          0               /* dummy for IP */
#define IPPROTO_UDP        17               /* user datagram protocol */
#define IPPROTO_TCP         6               /* tcp */

#define IP_TOS              7
#define IP_TTL              8
#define IP_MULTICAST_LOOP   9
#define IP_MULTICAST_TTL   10
#define IP_ADD_MEMBERSHIP  11
#define IP_DROP_MEMBERSHIP 12

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

struct in_addr {
	in_addr_t       s_addr;
};

struct sockaddr_in {
	sa_family_t     sin_family;
	in_port_t       sin_port;
	struct in_addr  sin_addr;
	unsigned char   sin_zero[8];
};

/* Request struct for multicast socket ops */
struct ip_mreq  {
	struct in_addr imr_multiaddr;	/* IP multicast address of group */
	struct in_addr imr_interface;	/* local IP address of interface */
};
