#include <stdlib.h>
#include <3ds.h>

static Handle amHandle = 0;

Result amInit()
{
	if(srvGetServiceHandle(&amHandle, "am:net") == 0)
		return (Result)0;
	else
		return srvGetServiceHandle(&amHandle, "am:u");
}

Result amExit()
{
	return svcCloseHandle(amHandle);
}

Result AM_GetTitleCount(u8 mediatype, u32 *count)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00010040;
	cmdbuf[1] = mediatype;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	*count = cmdbuf[2];
	
	return (Result)cmdbuf[1];
}

Result AM_GetTitleList(u8 mediatype, u32 count, void *buffer)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00020082;
	cmdbuf[1] = count;
	cmdbuf[2] = mediatype;
	cmdbuf[3] = ((count*8) << 4) | 12;
	cmdbuf[4] = (u32)buffer;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;
	
	return (Result)cmdbuf[1];
}

Result AM_GetDeviceId(u32 *deviceid)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000A0000;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	*deviceid = cmdbuf[3];
	
	return (Result)cmdbuf[1];
}

Result AM_StartCiaInstall(u8 mediatype, Handle *ciahandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x04020040;
	cmdbuf[1] = mediatype;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	*ciahandle = cmdbuf[3];
	
	return (Result)cmdbuf[1];
}

Result AM_StartDlpChildCiaInstall(Handle *ciahandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x04030000;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	*ciahandle = cmdbuf[3];
	
	return (Result)cmdbuf[1];
}

Result AM_CancelCIAInstall(Handle *ciahandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x04050002;
	cmdbuf[1] = 0x10;
	cmdbuf[2] = *ciahandle;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	return (Result)cmdbuf[1];
}

Result AM_FinishCiaInstall(u8 mediatype, Handle *ciahandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x04040002;
	cmdbuf[1] = 0x10;
	cmdbuf[2] = *ciahandle;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	return (Result)cmdbuf[1];
}

Result AM_DeleteTitle(u8 mediatype, u64 titleid)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x041000C0;
	cmdbuf[1] = mediatype;
	cmdbuf[2] = titleid & 0xffffffff;
	cmdbuf[3] = (titleid >> 32) & 0xffffffff;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	return (Result)cmdbuf[1];
}

Result AM_DeleteAppTitle(u8 mediatype, u64 titleid)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000400C0;
	cmdbuf[1] = mediatype;
	cmdbuf[2] = titleid & 0xffffffff;
	cmdbuf[3] = (titleid >> 32) & 0xffffffff;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	return (Result)cmdbuf[1];
}

Result AM_InstallFIRM(u8 mediatype, u64 titleid)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000400C0;
	cmdbuf[1] = mediatype;
	cmdbuf[2] = titleid & 0xffffffff;
	cmdbuf[3] = (titleid >> 32) & 0xffffffff;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	return (Result)cmdbuf[1];
}
