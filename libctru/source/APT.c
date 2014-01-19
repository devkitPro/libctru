#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/APT.h>
#include <ctr/svc.h>

void APT_GetLockHandle(Handle handle, u16 flags, Handle* lockHandle)
{
	u32* svcData=svc_getData();
	svcData[0]=0x10040; //request header code
	svcData[1]=flags;
	svc_sendSyncRequest(handle); //check return value...
	if(lockHandle)*lockHandle=svcData[1];
}

void APT_Initialize(Handle handle, u32 a, Handle* eventHandle1, Handle* eventHandle2)
{
	u32* svcData=svc_getData();
	svcData[0]=0x20080; //request header code
	svcData[1]=a;
	svcData[2]=0x0;
	svc_sendSyncRequest(handle); //check return value...
	if(eventHandle1)*eventHandle1=svcData[3]; //return to menu event ?
	if(eventHandle2)*eventHandle2=svcData[4];
}

Result APT_Enable(Handle handle, u32 a)
{
	u32* svcData=svc_getData();
	svcData[0]=0x30040; //request header code
	svcData[1]=a;
	svc_sendSyncRequest(handle); //check return value...
	return svcData[1];
}

u8 APT_InquireNotification(Handle handle, u32 appID)
{
	u32* svcData=svc_getData();
	svcData[0]=0xB0040; //request header code
	svcData[1]=appID;
	svc_sendSyncRequest(handle); //check return value...
	return svcData[2];
}

Result APT_PrepareToJumpToHomeMenu(Handle handle)
{
	u32* svcData=svc_getData();
	svcData[0]=0x2b0000; //request header code
	svc_sendSyncRequest(handle); //check return value...
	return svcData[1];
}

Result APT_JumpToHomeMenu(Handle handle, u32 a, u32 b, u32 c)
{
	u32* svcData=svc_getData();
	svcData[0]=0x2C0044; //request header code
	svcData[1]=a;
	svcData[2]=b;
	svcData[3]=c;
	svcData[4]=(b<<14)|2;
	svc_sendSyncRequest(handle); //check return value...
	return svcData[1];
}
