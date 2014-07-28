#include <3ds/types.h>
#include <3ds/srv.h>
#include <3ds/svc.h>

Handle srvHandle=0;

Result initSrv()
{
	Result ret=0;
	if(svcConnectToPort(&srvHandle, "srv:"))return ret;
	return srv_RegisterClient(&srvHandle);
}

Result exitSrv()
{
	if(srvHandle)svcCloseHandle(srvHandle);
}

Result srv_RegisterClient(Handle* handleptr)
{
	if(!handleptr)handleptr=&srvHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x10002; //request header code
	cmdbuf[1]=0x20;

	Result ret=0;
	if((ret=svcSendSyncRequest(*handleptr)))return ret;

	return cmdbuf[1];
}

Result srv_getServiceHandle(Handle* handleptr, Handle* out, char* server)
{
	if(!handleptr)handleptr=&srvHandle;
	u8 l=strlen(server);
	if(!out || !server || l>8)return -1;

	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=0x50100; //request header code
	strcpy((char*)&cmdbuf[1], server);
	cmdbuf[3]=l;
	cmdbuf[4]=0x0;

	Result ret=0;
	if((ret=svcSendSyncRequest(*handleptr)))return ret;

	*out=cmdbuf[3];

	return cmdbuf[1];
}
