#pragma once

#include <stdint.h>
#include <sys/socket.h>

#define INADDR_ANY		0x00000000
#define INADDR_BROADCAST	0xFFFFFFFF
#define INADDR_NONE		0xFFFFFFFF

#define INET_ADDRSTRLEN		16

//#define IPPROTO_IP		???
//#define IPPROTO_TCP		???
//#define IPPROTO_UDP		???

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

struct in_addr {
	in_addr_t	s_addr;
};

struct sockaddr_in {
	sa_family_t	sin_family;
	in_port_t	sin_port;
	struct in_addr	sin_addr;
	unsigned char	sin_zero[8];
};
