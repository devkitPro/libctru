#pragma once

#include <errno.h>
#include <string.h>
#include <sys/iosupport.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/services/soc.h>

#define SYNC_ERROR ENODEV

int __alloc_handle(int size);
__handle *__get_handle(int fd);
void __release_handle(int fd);

extern Handle	SOCU_handle;
extern Handle	socMemhandle;

static inline int
soc_get_fd(int fd)
{
	__handle *handle = __get_handle(fd);
	if(handle == NULL)
		return -ENODEV;
	if(strcmp(devoptab_list[handle->device]->name, "soc") != 0)
		return -ENOTSOCK;
	return *(Handle*)handle->fileStruct;
}

s32 _net_convert_error(s32 sock_retval);
