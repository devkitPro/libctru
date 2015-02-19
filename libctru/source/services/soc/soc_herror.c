#include "soc_common.h"
#include <netdb.h>
#include <stdio.h>

void herror(const char *s) {
	if(s)
		fiprintf(stderr, "%s\n", hstrerror(h_errno));
	else
		fiprintf(stderr, "%s: %s\n", s, hstrerror(h_errno));
}
