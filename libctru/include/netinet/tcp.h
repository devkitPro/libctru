#pragma once

#define SOL_TCP     6    /* TCP level */

enum{
	 _CTRU_TCP_OPT = 0x2000,            /* Flag for tcp opt values */
	 TCP_NODELAY   = 1 | _CTRU_TCP_OPT, /* Don't delay send to coalesce packets  */
	 TCP_MAXSEG    = 2 | _CTRU_TCP_OPT,
};
