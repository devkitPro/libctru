#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/HID.h>
#include <ctr/svc.h>

Result HIDUSER_GetInfo(Handle handle, Handle* outMemHandle)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0xa0000; //request header code

	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;

	if(outMemHandle)*outMemHandle=cmdbuf[3];

	return cmdbuf[1];
}

Result HIDUSER_Init(Handle handle)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x110000; //request header code

	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}
