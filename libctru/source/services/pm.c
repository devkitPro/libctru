#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/services/pm.h>

static Handle pmHandle;

Result pmInit()
{
	return srvGetServiceHandle(&pmHandle, "pm:app");	
}

Result pmExit()
{
	return svcCloseHandle(pmHandle);
}

Result PM_LaunchTitle(u8 mediatype, u64 titleid, u32 launch_flags)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00010140;
	cmdbuf[1] = titleid & 0xffffffff;
	cmdbuf[2] = (titleid >> 32) & 0xffffffff;
	cmdbuf[3] = mediatype;
	cmdbuf[4] = 0x0;
	cmdbuf[5] = launch_flags;
	
	if((ret = svcSendSyncRequest(pmHandle))!=0)return ret;
	
	return (Result)cmdbuf[1];
}

Result PM_GetTitleExheaderFlags(u8 mediatype, u64 titleid, u8* out)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00080100;
	cmdbuf[1] = titleid & 0xffffffff;
	cmdbuf[2] = (titleid >> 32) & 0xffffffff;
	cmdbuf[3] = mediatype;
	cmdbuf[4] = 0x0;
	
	if((ret = svcSendSyncRequest(pmHandle))!=0)return ret;
	
	memcpy(out, (u8*)(&cmdbuf[2]), 8);
	
	return (Result)cmdbuf[1];
}

Result PM_SetFIRMLaunchParams(u32 size, u8* in)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00090042;
	cmdbuf[1] = size;
	cmdbuf[2] = (size << 0x4) | 0xa;
	cmdbuf[3] = (u32)in;
	
	if((ret = svcSendSyncRequest(pmHandle))!=0)return ret;
		
	return (Result)cmdbuf[1];
}

Result PM_GetFIRMLaunchParams(u32 size, u8* out)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00070042;
	cmdbuf[1] = size;
	cmdbuf[2] = (size << 0x4) | 0xc;
	cmdbuf[3] = (u32)out;
	
	if((ret = svcSendSyncRequest(pmHandle))!=0)return ret;
		
	return (Result)cmdbuf[1];
}

Result PM_LaunchFIRMSetParams(u32 firm_titleid_low, u32 size, u8* in)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00020082;
	cmdbuf[1] = firm_titleid_low;
	cmdbuf[2] = size;
	cmdbuf[3] = (size << 0x4) | 0xa;
	cmdbuf[4] = (u32)in;
	
	if((ret = svcSendSyncRequest(pmHandle))!=0)return ret;
		
	return (Result)cmdbuf[1];
}