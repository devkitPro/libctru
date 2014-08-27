#include <stdlib.h>
#include <3ds.h>

static Handle ptmHandle;

Result ptmInit()
{
	return srvGetServiceHandle(&ptmHandle, "ptm:u");	
}

Result ptmExit()
{
	return svcCloseHandle(ptmHandle);
}

Result PTMU_GetBatteryLevel(Handle* servhandle, u8 *out)
{
	if(!servhandle)servhandle=&ptmHandle;
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00070000;

	if((ret = svcSendSyncRequest(*servhandle))!=0)return ret;

	*out = (u8)cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result PTMU_GetBatteryChargeState(Handle* servhandle, u8 *out)
{
	if(!servhandle)servhandle=&ptmHandle;
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00080000;

	if((ret = svcSendSyncRequest(*servhandle))!=0)return ret;

	*out = (u8)cmdbuf[2];

	return (Result)cmdbuf[1];
}
