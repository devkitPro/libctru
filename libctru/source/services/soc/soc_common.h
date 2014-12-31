#pragma once

#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/services/soc.h>

extern Handle	SOCU_handle;
extern int	SOCU_errno;
extern Handle	socMemhandle;

s32 _net_convert_error(s32 sock_retval);
