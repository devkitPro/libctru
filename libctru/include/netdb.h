#pragma once

#include <netinet/in.h>

#define HOST_NOT_FOUND	1
#define NO_DATA		2
#define NO_ADDRESS	NO_DATA
#define NO_RECOVERY	3
#define TRY_AGAIN	4

struct hostent {
	char	*h_name;
	char	**h_aliases;
	int	h_addrtype;
	int	h_length;
	char	**h_addr_list;
	char	*h_addr;
};


#define AI_PASSIVE     0x01
#define AI_CANONNAME   0x02
#define AI_NUMERICHOST 0x04
#define AI_NUMERICSERV 0x00 /* probably 0x08 but services names are never resolved */

// doesn't apply to 3ds
#define AI_ADDRCONFIG  0x00


#define NI_MAXHOST     1025
#define NI_MAXSERV       32

#define NI_NOFQDN      0x01
#define NI_NUMERICHOST 0x02
#define NI_NAMEREQD    0x04
#define NI_NUMERICSERV 0x00 /* probably 0x08 but services names are never resolved */
#define NI_DGRAM       0x00 /* probably 0x10 but services names are never resolved */

#define EAI_FAMILY   (-303)
#define EAI_MEMORY   (-304)
#define EAI_NONAME   (-305)
#define EAI_SOCKTYPE (-307)

struct addrinfo {
	int             ai_flags;
	int             ai_family;
	int             ai_socktype;
	int             ai_protocol;
	socklen_t       ai_addrlen;
	char            *ai_canonname;
	struct sockaddr *ai_addr;
	struct addrinfo *ai_next;
};

#ifdef __cplusplus
extern "C" {
#endif

	extern int	h_errno;
	struct hostent*	gethostbyname(const char *name);
	struct hostent*	gethostbyaddr(const void *addr, socklen_t len, int type);
	void		herror(const char *s);
	const char*	hstrerror(int err);

	int getnameinfo(const struct sockaddr *sa, socklen_t salen,
		char *host, socklen_t hostlen,
		char *serv, socklen_t servlen, int flags);

	int getaddrinfo(const char *node, const char *service,
		const struct addrinfo *hints,
		struct addrinfo **res);

	void freeaddrinfo(struct addrinfo *ai);

	const char *gai_strerror(int ecode);
#ifdef __cplusplus
}
#endif
