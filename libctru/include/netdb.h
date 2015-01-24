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
};

#ifdef __cplusplus
extern "C" {
#endif

	extern int	h_errno;
	struct hostent*	gethostbyname(const char *name);
	void		herror(const char *s);
	const char*	hstrerror(int err);

#ifdef __cplusplus
}
#endif
