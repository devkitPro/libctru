#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/services/am.h>

static Handle amHandle = 0;

Result amInit()
{
	if(srvGetServiceHandle(&amHandle, "am:net") == 0)
		return (Result)0;
	else if(srvGetServiceHandle(&amHandle, "am:u") == 0)
		return (Result)0;
	else if(srvGetServiceHandle(&amHandle, "am:sys") == 0)
		return (Result)0;
	else return srvGetServiceHandle(&amHandle, "am:app");
}

Result amExit()
{
	return svcCloseHandle(amHandle);
}

Handle *amGetSessionHandle()
{
	return &amHandle;
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

Result AM_GetTitleIdList(u8 mediatype, u32 count, u64 *titleIDs)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00020082;
	cmdbuf[1] = count;
	cmdbuf[2] = mediatype;
	cmdbuf[3] = ((count*8) << 4) | 12;
	cmdbuf[4] = (u32)titleIDs;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;
	
	return (Result)cmdbuf[1];
}

Result AM_ListTitles(u8 mediatype, u32 titleCount, u64 *titleIdList, TitleList *titleList)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00030084;
	cmdbuf[1] = mediatype;
	cmdbuf[2] = titleCount;
	cmdbuf[3] = ((titleCount*8)<<4) | 10;
	cmdbuf[4] = (u32)titleIdList;
	cmdbuf[5] = ((sizeof(TitleList)*titleCount)<<4) | 12;
	cmdbuf[6] = (u32)titleList;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	return (Result)cmdbuf[1];
}

Result AM_GetDeviceId(u32 *deviceID)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000A0000;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	*deviceID = cmdbuf[3];
	
	return (Result)cmdbuf[1];
}

Result AM_StartCiaInstall(u8 mediatype, Handle *ciaHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x04020040;
	cmdbuf[1] = mediatype;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	*ciaHandle = cmdbuf[3];
	
	return (Result)cmdbuf[1];
}

Result AM_StartDlpChildCiaInstall(Handle *ciaHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x04030000;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	*ciaHandle = cmdbuf[3];
	
	return (Result)cmdbuf[1];
}

Result AM_CancelCIAInstall(Handle *ciaHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x04040002;
	cmdbuf[1] = 0x10;
	cmdbuf[2] = *ciaHandle;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	return (Result)cmdbuf[1];
}

Result AM_FinishCiaInstall(u8 mediatype, Handle *ciaHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x04050002;
	cmdbuf[1] = 0x10;
	cmdbuf[2] = *ciaHandle;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	return (Result)cmdbuf[1];
}

Result AM_DeleteTitle(u8 mediatype, u64 titleID)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x041000C0;
	cmdbuf[1] = mediatype;
	cmdbuf[2] = titleID & 0xffffffff;
	cmdbuf[3] = (u32)(titleID >> 32);

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	return (Result)cmdbuf[1];
}

Result AM_DeleteAppTitle(u8 mediatype, u64 titleID)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000400C0;
	cmdbuf[1] = mediatype;
	cmdbuf[2] = titleID & 0xffffffff;
	cmdbuf[3] = (u32)(titleID >> 32);

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	return (Result)cmdbuf[1];
}

Result AM_InstallNativeFirm()
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x040F0000;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	return (Result)cmdbuf[1];
}

Result AM_GetTitleProductCode(u8 mediatype, u64 titleID, char* productCode)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = 0x000500C0;
	cmdbuf[1] = mediatype;
	cmdbuf[2] = titleID & 0xffffffff;
	cmdbuf[3] = (u32)(titleID >> 32);
	
	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	// The product code string can use the full 16 bytes without NULL terminator
	if(productCode) snprintf(productCode, 16, "%s", (char*)&cmdbuf[2]);

	return (Result)cmdbuf[1];
}

Result AM_GetCiaFileInfo(u8 mediatype, TitleList *titleEntry, Handle fileHandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = 0x04080042;
	cmdbuf[1] = mediatype;
	cmdbuf[2] = 0;
	cmdbuf[3] = fileHandle;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	if(titleEntry) memcpy(titleEntry, &cmdbuf[2], sizeof(TitleList));

	return (Result)cmdbuf[1];
}
