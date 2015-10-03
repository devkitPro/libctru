#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/services/ptm.h>
#include <3ds/ipc.h>


static Handle ptmHandle;

Result ptmInit(void)
{
	return srvGetServiceHandle(&ptmHandle, "ptm:u");	
}

Result ptmExit(void)
{
	return svcCloseHandle(ptmHandle);
}

Result PTMU_GetShellState(Handle* servhandle, u8 *out)
{
	if(!servhandle)servhandle=&ptmHandle;
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6,0,0); // 0x60000

	if((ret = svcSendSyncRequest(*servhandle))!=0)return ret;

	*out = (u8)cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result PTMU_GetBatteryLevel(Handle* servhandle, u8 *out)
{
	if(!servhandle)servhandle=&ptmHandle;
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x7,0,0); // 0x70000

	if((ret = svcSendSyncRequest(*servhandle))!=0)return ret;

	*out = (u8)cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result PTMU_GetBatteryChargeState(Handle* servhandle, u8 *out)
{
	if(!servhandle)servhandle=&ptmHandle;
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x8,0,0); // 0x80000

	if((ret = svcSendSyncRequest(*servhandle))!=0)return ret;

	*out = (u8)cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result PTMU_GetPedometerState(Handle* servhandle, u8 *out)
{
	if(!servhandle)servhandle=&ptmHandle;
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x9,0,0); // 0x90000

	if((ret = svcSendSyncRequest(*servhandle))!=0)return ret;

	*out = (u8)cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result PTMU_GetTotalStepCount(Handle* servhandle, u32 *steps)
{
	if(!servhandle)servhandle=&ptmHandle;
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xC,0,0); // 0xC0000

	if((ret = svcSendSyncRequest(*servhandle))!=0)return ret;

	*steps = cmdbuf[2];

	return (Result)cmdbuf[1];
}
