#include "soc_common.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <stdint.h>

int inet_aton(const char *cp, struct in_addr *inp)
{
	int      base;
	uint32_t val;
	int      c;
	char     bytes[4];
	size_t   num_bytes = 0;

	c = *cp;
	for(;;) {
		if(!isdigit(c)) return 0;

		val = 0;
		base = 10;
		if(c == '0') {
			c = *++cp;
			if(c == 'x' || c == 'X') {
				base = 16;
				c = *++cp;
			}
			else base = 8;
		}

		for(;;) {
			if(isdigit(c)) {
				if(base == 8 && c >= '8') return 0;
				val *= base;
				val += c - '0';
				c    = *++cp;
			}
			else if(base == 16 && isxdigit(c)) {
				val *= base;
				val += c + 10 - (islower(c) ? 'a' : 'A');
				c    = *++cp;
			}
			else break;
		}

		if(c == '.') {
			if(num_bytes > 3) return 0;
			if(val > 0xFF) return 0;
			bytes[num_bytes++] = val;
			c = *++cp;
		}
		else break;
	}

	if(c != 0) return 0;

	switch(num_bytes) {
	case 0:
		break;

	case 1:
		if(val > 0xFFFFFF) return 0;
		val |= bytes[0] << 24;
		break;

	case 2:
		if(val > 0xFFFF) return 0;
		val |= bytes[0] << 24;
		val |= bytes[1] << 16;
		break;

	case 3:
		if(val > 0xFF) return 0;
		val |= bytes[0] << 24;
		val |= bytes[1] << 16;
		val |= bytes[2] << 8;
		break;
	}

	if(inp)
		inp->s_addr = htonl(val);

	return 1;
}
