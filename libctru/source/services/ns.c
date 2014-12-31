#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/services/ns.h>

static Handle nsHandle;

Result nsInit()
{
	return srvGetServiceHandle(&nsHandle, "ns:s");	
}

Result nsExit()
{
	return svcCloseHandle(nsHandle);
}

Result NS_LaunchTitle(u64 titleid, u32 launch_flags, u32 *procid)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000200C0;
	cmdbuf[1] = titleid & 0xffffffff;
	cmdbuf[2] = (titleid >> 32) & 0xffffffff;
	cmdbuf[3] = launch_flags;
	
	if((ret = svcSendSyncRequest(nsHandle))!=0)return ret;

	if(procid != NULL)
		*procid = cmdbuf[2];
	
	return (Result)cmdbuf[1];
}

Result NS_RebootToTitle(u8 mediatype, u64 titleid)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00100180;
	cmdbuf[1] = 0x1;
	cmdbuf[2] = titleid & 0xffffffff;
	cmdbuf[3] = (titleid >> 32) & 0xffffffff;
	cmdbuf[4] = mediatype;
	cmdbuf[5] = 0x0; // reserved
	cmdbuf[6] = 0x0;
	
	if((ret = svcSendSyncRequest(nsHandle))!=0)return ret;

	return (Result)cmdbuf[1];
}