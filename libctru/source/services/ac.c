#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/services/ac.h>
#include <3ds/ipc.h>

static Handle acHandle;

Result acInit(void)
{
	Result ret = srvGetServiceHandle(&acHandle, "ac:u");
	if(!ret)return ret;
	return srvGetServiceHandle(&acHandle, "ac:i");
}

Result acExit(void)
{
	return svcCloseHandle(acHandle);
}

// ptr=0x200-byte outbuf
Result ACU_CreateDefaultConfig(Handle* servhandle, u32 *ptr)
{
	if(!servhandle)servhandle=&acHandle;
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 savedValue0 = staticbufs[0];
	u32 savedValue1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x1,0,0); // 0x00010000
	staticbufs[0] = IPC_Desc_StaticBuffer(0x200,0);
	staticbufs[1] = (u32)ptr;

	if((ret = svcSendSyncRequest(*servhandle))!=0)return ret;

	staticbufs[0] = savedValue0;
	staticbufs[1] = savedValue1;

	return (Result)cmdbuf[1];
}

// Unknown what this cmd does at the time of writing. (ptr=0x200-byte inbuf/outbuf)
Result ACU_cmd26(Handle* servhandle, u32 *ptr, u8 val)
{
	if(!servhandle)servhandle=&acHandle;
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 savedValue0 = staticbufs[0];
	u32 savedValue1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x26,1,2); // 0x00260042
	cmdbuf[1] = (u32)val;
	staticbufs[0] = IPC_Desc_StaticBuffer(0x200,0);
	staticbufs[1] = (u32)ptr;
	cmdbuf[2] = IPC_Desc_StaticBuffer(0x200,0);
	cmdbuf[3] = (u32)ptr;

	if((ret = svcSendSyncRequest(*servhandle))!=0)return ret;

	staticbufs[0] = savedValue0;
	staticbufs[1] = savedValue1;

	return (Result)cmdbuf[1];
}

Result ACU_GetWifiStatus(Handle* servhandle, u32 *out)
{
	if(!servhandle)servhandle=&acHandle;
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xD,0,0); // 0x000D0000

	if((ret = svcSendSyncRequest(*servhandle))!=0)return ret;

	*out = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result ACU_WaitInternetConnection(void)
{
	Handle servhandle = 0;
	Result ret=0;
	u32 outval=0;

	if((ret = srvGetServiceHandle(&servhandle, "ac:u"))!=0)return ret;

	while(1)
	{
		ret = ACU_GetWifiStatus(&servhandle, &outval);
		if(ret==0 && outval!=0)break;
	}

	svcCloseHandle(servhandle);

	return ret;
}
