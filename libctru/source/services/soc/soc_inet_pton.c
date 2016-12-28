#include "soc_common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>


static int inet_pton4(const char *restrict src, void *restrict dst)
{
	u8 ip[4] ALIGN(4);
	if(sscanf(src,"%hhu.%hhu.%hhu.%hhu",&ip[0], &ip[1], &ip[2], &ip[3]) != 4) return 0;

	memcpy(dst,ip,4);
	return 1;
}


int inet_pton(int af, const char *restrict src, void *restrict dst)
{
	if(af == AF_INET)
	{
		return inet_pton4(src,dst);
	}
	// only support IPv4
	errno = EAFNOSUPPORT;
	return -1;
}
