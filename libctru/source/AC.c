#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>

#include <ctr/AC.h>
#include <ctr/svc.h>
#include <ctr/srv.h>

Result ACU_cmd1(Handle servhandle, u32 *ptr)//Unknown what this cmd does at the time of writing. (ptr=0x200-byte outbuf)
{
	u32 tmp0, tmp1;
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	tmp0 = cmdbuf[0x100>>2];
	tmp1 = cmdbuf[0x104>>2];

	cmdbuf[0] = 0x00010000;
	cmdbuf[0x100>>2] = 0x00800002;
	cmdbuf[0x104>>2] = (u32)ptr;

	if((ret = svc_sendSyncRequest(servhandle))!=0)return ret;

	cmdbuf[0x100>>2] = tmp0;
	cmdbuf[0x104>>2] = tmp1;

	return (Result)cmdbuf[1];
}

Result ACU_cmd26(Handle servhandle, u32 *ptr, u8 val)//Unknown what this cmd does at the time of writing. (ptr=0x200-byte inbuf/outbuf)
{
	u32 tmp0, tmp1;
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	tmp0 = cmdbuf[0x100>>2];
	tmp1 = cmdbuf[0x104>>2];

	cmdbuf[0] = 0x00260042;
	cmdbuf[1] = (u32)val;
	cmdbuf[0x100>>2] = 0x00800002;
	cmdbuf[0x104>>2] = (u32)ptr;
	cmdbuf[2] = 0x00800002;
	cmdbuf[3] = (u32)ptr;

	if((ret = svc_sendSyncRequest(servhandle))!=0)return ret;

	cmdbuf[0x100>>2] = tmp0;
	cmdbuf[0x104>>2] = tmp1;

	return (Result)cmdbuf[1];
}

Result ACU_GetWifiStatus(Handle servhandle, u32 *out)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000D0000;

	if((ret = svc_sendSyncRequest(servhandle))!=0)return ret;

	*out = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result ACU_WaitInternetConnection()
{
	Handle servhandle = 0;
	Result ret=0;
	u32 outval=0;

	if((ret = srv_getServiceHandle(NULL, &servhandle, "ac:u"))!=0)return ret;

	while(1)
	{
		ret = ACU_GetWifiStatus(servhandle, &outval);
		if(ret==0 && outval==1)break;
	}

	svc_closeHandle(servhandle);

	return ret;
}

