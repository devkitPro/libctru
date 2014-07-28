#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/HID.h>
#include <3ds/srv.h>
#include <3ds/svc.h>

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
	svcMapMemoryBlock(hidMemHandle, (u32)hidSharedMem, 0x1, 0x10000000);

	if((ret=HIDUSER_EnableAccelerometer(NULL)))return ret;

	return 0;
}

void hidExit()
{
	svcUnmapMemoryBlock(hidMemHandle, (u32)hidSharedMem);
	svcCloseHandle(hidMemHandle);
	svcCloseHandle(hidHandle);
}

Result HIDUSER_GetInfo(Handle* handle, Handle* outMemHandle)
{
	if(!handle)handle=&hidHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0xa0000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	if(outMemHandle)*outMemHandle=cmdbuf[3];

	return cmdbuf[1];
}

Result HIDUSER_EnableAccelerometer(Handle* handle)
{
	if(!handle)handle=&hidHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x110000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}
