#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/srv.h>
#include <ctr/svc.h>

Result srv_10002(Handle handle)
{
	u32* svcData=svc_getData();
	svcData[0]=0x10002; //request header code
	svcData[1]=0x20;
	svc_sendSyncRequest(handle); //check return value...
	return svcData[1];
}

void getSrvHandle(Handle* out)
{
	if(!out)return;

	svc_connectToPort(out, "srv:");
	srv_10002(*out);
}

void srv_getServiceHandle(Handle handle, Handle* out, char* server)
{
	u8 l=strlen(server);
	if(!out || !server || l>8)return;
	u32* svcData=svc_getData();
	svcData[0]=0x50100; //request header code
	strcpy((char*)&svcData[1], server);
	svcData[3]=l;
	svcData[4]=0x0;
	svc_sendSyncRequest(handle); //check return value...
	*out=svcData[3];
}
