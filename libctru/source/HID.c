#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/HID.h>
#include <ctr/srv.h>
#include <ctr/svc.h>

Handle hidHandle;
Handle hidMemHandle;

vu32* hidSharedMem;

Result hidInit(u32* sharedMem)
{
	if(!sharedMem)sharedMem=(u32*)HID_SHAREDMEM_DEFAULT;
	Result ret=0;

	if((ret=srv_getServiceHandle(NULL, &hidHandle, "hid:USER")))return ret;
	
	if((ret=HIDUSER_GetInfo(NULL, &hidMemHandle)))return ret;
	hidSharedMem=sharedMem;
	svc_mapMemoryBlock(hidMemHandle, (u32)hidSharedMem, 0x1, 0x10000000);

	if((ret=HIDUSER_EnableAccelerometer(NULL)))return ret;

	return 0;
}

void hidExit()
{
	svc_unmapMemoryBlock(hidMemHandle, (u32)hidSharedMem);
	svc_closeHandle(hidMemHandle);
	svc_closeHandle(hidHandle);
}

Result HIDUSER_GetInfo(Handle* handle, Handle* outMemHandle)
{
	if(!handle)handle=&hidHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0xa0000; //request header code

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	if(outMemHandle)*outMemHandle=cmdbuf[3];

	return cmdbuf[1];
}

Result HIDUSER_EnableAccelerometer(Handle* handle)
{
	if(!handle)handle=&hidHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x110000; //request header code

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}
