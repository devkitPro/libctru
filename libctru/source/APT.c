#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/APT.h>
#include <ctr/svc.h>

Result APT_GetLockHandle(Handle handle, u16 flags, Handle* lockHandle)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x10040; //request header code
	cmdbuf[1]=flags;
	
	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;
	
	if(lockHandle)*lockHandle=cmdbuf[5];
	
	return cmdbuf[1];
}

Result APT_Initialize(Handle handle, NS_APPID appId, Handle* eventHandle1, Handle* eventHandle2)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x20080; //request header code
	cmdbuf[1]=appId;
	cmdbuf[2]=0x0;
	
	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;
	
	if(eventHandle1)*eventHandle1=cmdbuf[3]; //return to menu event ?
	if(eventHandle2)*eventHandle2=cmdbuf[4];

	return cmdbuf[1];
}

Result APT_Enable(Handle handle, u32 a)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x30040; //request header code
	cmdbuf[1]=a;
	
	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;
	
	return cmdbuf[1];
}

Result APT_InquireNotification(Handle handle, u32 appID, u8* signalType)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0xB0040; //request header code
	cmdbuf[1]=appID;
	
	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;

	if(signalType)*signalType=cmdbuf[2];
	
	return cmdbuf[1];
}

Result APT_PrepareToJumpToHomeMenu(Handle handle)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x2b0000; //request header code
	
	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;
	
	return cmdbuf[1];
}

Result APT_JumpToHomeMenu(Handle handle, u32 a, u32 b, u32 c)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x2C0044; //request header code
	cmdbuf[1]=a;
	cmdbuf[2]=b;
	cmdbuf[3]=c;
	cmdbuf[4]=(b<<14)|2;
	
	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;
	
	return cmdbuf[1];
}

Result APT_NotifyToWait(Handle handle, u32 a)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x430040; //request header code
	cmdbuf[1]=a;
	
	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}
