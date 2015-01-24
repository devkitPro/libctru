#include "soc_common.h"
#include <netdb.h>

static const struct
{
	int        err;
	const char *str;
} error_strings[] =
{
	{ 0,              "Resolver Error 0 (no error)",     },
	{ HOST_NOT_FOUND, "Unknown host",                    },
	{ NO_DATA,        "No address associated with name", },
	{ NO_RECOVERY,    "Unknown server error",            },
	{ TRY_AGAIN,      "Host name lookup failure",        },
};
static const size_t num_error_strings = sizeof(error_strings)/sizeof(error_strings[0]);

const char* hstrerror(int err) {
	size_t i;
	for(i = 0; i < num_error_strings; ++i) {
		if(error_strings[i].err == err)
			return error_strings[i].str;
	}

	return "Unknown resolver error";
}
