#pragma once

#include <stdint.h>
#include <sys/time.h>

#define SOL_SOCKET	0xFFFF

#define PF_UNSPEC	0
#define PF_INET		2
#define PF_INET6	10

#define AF_UNSPEC	PF_UNSPEC
#define AF_INET		PF_INET
#define AF_INET6	PF_INET6

#define SOCK_STREAM	1
#define SOCK_DGRAM	2

#define MSG_CTRUNC	0x01000000
#define MSG_DONTROUTE	0x02000000
#define MSG_EOR		0x04000000
#define MSG_OOB		0x08000000
#define MSG_PEEK	0x10000000
#define MSG_TRUNC	0x20000000
#define MSG_WAITALL	0x40000000

#define SHUT_RD		0
#define SHUT_WR		1
#define SHUT_RDWR	2

#define SO_DEBUG	0x0001
#define SO_ACCEPTCONN	0x0002
#define SO_REUSEADDR	0x0004
#define SO_KEEPALIVE	0x0008
#define SO_DONTROUTE	0x0010
#define SO_BROADCAST	0x0020
#define SO_USELOOPBACK	0x0040
#define SO_LINGER	0x0080
#define SO_OOBINLINE	0x0100
#define SO_REUSEPORT	0x0200
#define SO_SNDBUF	0x1001
#define SO_RCVBUF	0x1002
#define SO_SNDLOWAT	0x1003
#define SO_RCVLOWAT	0x1004
#define SO_SNDTIMEO	0x1005
#define SO_RCVTIMEO	0x1006
#define SO_ERROR	0x1007
#define SO_TYPE		0x1008

typedef uint32_t socklen_t;
typedef uint16_t sa_family_t;

struct sockaddr {
	sa_family_t	sa_family;
	char		sa_data[];
};

struct sockaddr_storage {
	sa_family_t	ss_family;
	char		__ss_padding[14];
};

struct linger {
	int l_onoff;
	int l_linger;
};

#ifdef __cplusplus
extern "C" {
#endif

	int	accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
	int	bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	int	closesocket(int sockfd);
	int	connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	int	getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
	int	getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
	int	getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
	int	listen(int sockfd, int backlog);
	ssize_t	recv(int sockfd, void *buf, size_t len, int flags);
	ssize_t	recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
	ssize_t	send(int sockfd, const void *buf, size_t len, int flags);
	ssize_t	sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
	int	setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
	int	shutdown(int sockfd, int how);
	int	socket(int domain, int type, int protocol);
	int	sockatmark(int sockfd);

#ifdef __cplusplus
}
#endif
