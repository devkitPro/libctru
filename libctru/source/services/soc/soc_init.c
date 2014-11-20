#include "soc_common.h"

static Result socu_cmd1(Handle memhandle, u32 memsize)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00010044;
	cmdbuf[1] = memsize;
	cmdbuf[2] = 0x20;
	cmdbuf[4] = 0;
	cmdbuf[5] = memhandle;

	if((ret = svcSendSyncRequest(SOCU_handle))!=0)return ret;

	return cmdbuf[1];
}

Result SOC_Initialize(u32 *context_addr, u32 context_size)
{
	Result ret=0;

	ret = svcCreateMemoryBlock(&socMemhandle, (u32)context_addr, context_size, 0, 3);
	if(ret!=0)return ret;

	if((ret = srvGetServiceHandle(&SOCU_handle, "soc:U"))!=0)return ret;

	return socu_cmd1(socMemhandle, context_size);
}

Result SOC_Shutdown(void)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00190000;

	if((ret = svcSendSyncRequest(SOCU_handle))!=0)return ret;

	svcCloseHandle(SOCU_handle);
	svcCloseHandle(socMemhandle);

	return cmdbuf[1];
}
