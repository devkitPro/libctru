#ifndef NETINET_IN_H
#define NETINET_IN_H

#include "sys/socket.h"

#define INADDR_ANY		0x00000000
#define INADDR_BROADCAST	0xFFFFFFFF
#define INADDR_NONE		0xFFFFFFFF

struct in_addr {
	unsigned long s_addr;
};

struct sockaddr_in {
	unsigned short		sin_family;
	unsigned short		sin_port;
	struct in_addr		sin_addr;
	unsigned char		sin_zero[8];
};

#ifdef __cplusplus
extern "C" {
#endif

	// actually from arpa/inet.h - but is included through netinet/in.h
	unsigned long inet_addr(const char *cp);
	int inet_aton(const char *cp, struct in_addr *inp);
	char *inet_ntoa(struct in_addr in);

#ifdef __cplusplus
};
#endif

#endif // NETINET_IN_H
