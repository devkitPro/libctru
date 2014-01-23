#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/srv.h>
#include <ctr/svc.h>

Result srv_Initialize(Handle handle)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x10002; //request header code
	cmdbuf[1]=0x20;
	svc_sendSyncRequest(handle); //check return value...
	return cmdbuf[1];
}

void getSrvHandle(Handle* out)
{
	if(!out)return;

	svc_connectToPort(out, "srv:");
	srv_Initialize(*out);
}

void srv_getServiceHandle(Handle handle, Handle* out, char* server)
{
	u8 l=strlen(server);
	if(!out || !server || l>8)return;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x50100; //request header code
	strcpy((char*)&cmdbuf[1], server);
	cmdbuf[3]=l;
	cmdbuf[4]=0x0;
	svc_sendSyncRequest(handle); //check return value...
	*out=cmdbuf[3];
}
