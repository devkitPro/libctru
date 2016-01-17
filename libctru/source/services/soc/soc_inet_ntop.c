#include "soc_common.h"
#include <arpa/inet.h>
#include <stdio.h>


static const char *inet_ntop4(const void *restrict src, char *restrict dst, socklen_t size)
{
	const u8 * ip = src;
	if(size < INET_ADDRSTRLEN)
	{
		errno = ENOSPC;
		return NULL;
	}
	snprintf(dst,size,"%hhu.%hhu.%hhu.%hhu",ip[0], ip[1], ip[2], ip[3]);
	return dst;
}


const char *inet_ntop(int af, const void *restrict src, char *restrict dst, socklen_t size)
{
	if(af == AF_INET)
	{
		return inet_ntop4(src,dst,size);
	}
	// only support IPv4
	errno = EAFNOSUPPORT;
	return NULL;
}
