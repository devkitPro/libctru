#include "soc_common.h"
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>

int fcntl(int fd, int cmd, ...)
{
	int ret=0;
	int arg=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	va_list args;

	if(cmd!=F_GETFL && cmd!=F_SETFL)
	{
		SOCU_errno = -EINVAL;
		return -1;
	}

	va_start(args, cmd);
	if(cmd==F_SETFL)
	{
		arg = va_arg(args, int);

		if(arg && arg!=O_NONBLOCK)
		{
			SOCU_errno = -EINVAL;
			va_end(args);
			return -1;
		}

		if(arg==O_NONBLOCK)arg = 0x4;
	}
	va_end(args);

	cmdbuf[0] = 0x001300C2;
	cmdbuf[1] = (u32)fd;
	cmdbuf[2] = (u32)cmd;
	cmdbuf[3] = (u32)arg;
	cmdbuf[4] = 0x20;

	if((ret = svcSendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret<0)return -1;
	return ret;
}
