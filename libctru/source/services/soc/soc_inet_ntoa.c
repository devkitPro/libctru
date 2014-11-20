#include "soc_common.h"
#include <arpa/inet.h>
#include <netinet/in.h>

static char buffer[INET_ADDRSTRLEN];

char* inet_ntoa(struct in_addr in)
{
	unsigned char *addrbuf = (unsigned char*)&in;
	char *p;
	size_t i;
	unsigned int n;

	memset(buffer, 0, sizeof(buffer));
	for(p = buffer, i = 0; i < 4; ++i) {
		if(i > 0) *p++ = '.';

		n = addrbuf[i];
		if(n >= 100) {
			*p++ = n/100 + '0';
			n %= 100;
		}
		if(n >= 10 || addrbuf[i] >= 100) {
			*p++ = n/10 + '0';
			n %= 10;
		}
		*p++ = n + '0';
	}
	*p = 0;

	return buffer;
}
