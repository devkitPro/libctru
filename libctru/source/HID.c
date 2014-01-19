#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/HID.h>
#include <ctr/svc.h>

void HIDUSER_GetInfo(Handle handle, Handle* outMemHandle)
{
	u32* svcData=svc_getData();
	svcData[0]=0xa0000; //request header code
	svc_sendSyncRequest(handle); //check return value...
	if(outMemHandle)*outMemHandle=svcData[3];
}

void HIDUSER_Init(Handle handle)
{
	u32* svcData=svc_getData();
	svcData[0]=0x110000; //request header code
	svc_sendSyncRequest(handle); //check return value...
}
