#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/ac.h>
#include <3ds/ipc.h>

static Handle acHandle;
static int acRefCount;

Result acInit(void)
{
	Result ret;

	if (AtomicPostIncrement(&acRefCount)) return 0;

	// ac:i has the most commands, then ac:u
	ret = srvGetServiceHandle(&acHandle, "ac:i");
	if(R_FAILED(ret)) ret = srvGetServiceHandle(&acHandle, "ac:u");
	if(R_FAILED(ret)) AtomicDecrement(&acRefCount);

	return ret;
}

void acExit(void)
{
	if (AtomicDecrement(&acRefCount)) return;
	svcCloseHandle(acHandle);
}

Result acWaitInternetConnection(void)
{
	Result ret = 0;
	u32 status = 0;
	while(R_SUCCEEDED(ret = ACU_GetWifiStatus(&status)) && status == 0);
	return ret;
}

Result ACU_CreateDefaultConfig(acuConfig* config)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	cmdbuf[0] = IPC_MakeHeader(0x1,0,0); // 0x10000
	staticbufs[0] = IPC_Desc_StaticBuffer(0x200, 0);
	staticbufs[1] = (u32)config;

	if(R_FAILED(ret = svcSendSyncRequest(acHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACU_ConnectAsync(const acuConfig* config, Handle connectionHandle)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4,0,6); // 0x40006
	cmdbuf[1] = 0x20;
	cmdbuf[3] = 0;
	cmdbuf[4] = connectionHandle;
	cmdbuf[5] = IPC_Desc_StaticBuffer(0x200, 1);
	cmdbuf[6] = (u32)config;

	if(R_FAILED(ret = svcSendSyncRequest(acHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACU_GetLastErrorCode(u32* errorCode)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xA,0,0); // 0xA0000

	if(R_FAILED(ret = svcSendSyncRequest(acHandle)))return ret;

	if(errorCode) *errorCode = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result ACU_GetLastDetailErrorCode(u32* errorCode)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xB,0,0); // 0xB0000

	if(R_FAILED(ret = svcSendSyncRequest(acHandle)))return ret;

	if(errorCode) *errorCode = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result ACU_GetStatus(u32 *out)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xC,0,0); // 0xC0000

	if(R_FAILED(ret = svcSendSyncRequest(acHandle)))return ret;

	*out = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result ACU_GetWifiStatus(u32 *out)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xD,0,0); // 0xD0000

	if(R_FAILED(ret = svcSendSyncRequest(acHandle)))return ret;

	*out = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result ACU_SetAllowApType(acuConfig* config, u8 type)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	cmdbuf[0] = IPC_MakeHeader(0x22,1,2); // 0x220042
	cmdbuf[1] = type;
	cmdbuf[2] = IPC_Desc_StaticBuffer(0x200, 0);
	cmdbuf[3] = (u32)config;
	staticbufs[0] = IPC_Desc_StaticBuffer(0x200, 0);
	staticbufs[1] = (u32)config;

	if(R_FAILED(ret = svcSendSyncRequest(acHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACU_SetNetworkArea(acuConfig* config, u8 area)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	cmdbuf[0] = IPC_MakeHeader(0x25,1,2); // 0x250042
	cmdbuf[1] = area;
	cmdbuf[2] = IPC_Desc_StaticBuffer(0x200, 0);
	cmdbuf[3] = (u32)config;
	staticbufs[0] = IPC_Desc_StaticBuffer(0x200, 0);
	staticbufs[1] = (u32)config;

	if(R_FAILED(ret = svcSendSyncRequest(acHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACU_SetRequestEulaVersion(acuConfig* config)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	cmdbuf[0] = IPC_MakeHeader(0x2D,2,2); // 0x2D0082
	cmdbuf[1] = 0;
	cmdbuf[2] = 0;
	cmdbuf[3] = IPC_Desc_StaticBuffer(0x200, 0);
	cmdbuf[4] = (u32)config;
	staticbufs[0] = IPC_Desc_StaticBuffer(0x200, 0);
	staticbufs[1] = (u32)config;

	if(R_FAILED(ret = svcSendSyncRequest(acHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACU_GetProxyPassword(char *password)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	cmdbuf[0] = IPC_MakeHeader(0x3B,0,0); // 0x3B0000

	staticbufs[0] = IPC_Desc_StaticBuffer(0x20, 0);
	staticbufs[1] = (u32)password;

	if(R_FAILED(ret = svcSendSyncRequest(acHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result ACU_GetSecurityMode(acSecurityMode *mode)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x33,0,0); // 0x330000

	if(R_FAILED(ret = svcSendSyncRequest(acHandle)))return ret;

	*mode = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result ACU_GetSSID(char *SSID)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	cmdbuf[0] = IPC_MakeHeader(0x34,0,0); // 0x340000

	staticbufs[0] = IPC_Desc_StaticBuffer(0x20, 0);
	staticbufs[1] = (u32)SSID;

	if(R_FAILED(ret = svcSendSyncRequest(acHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result ACU_GetSSIDLength(u32 *out)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x35,0,0); // 0x350000

	if(R_FAILED(ret = svcSendSyncRequest(acHandle)))return ret;

	*out = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result ACU_GetProxyEnable(bool *enable)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x36,0,0); // 0x360000

	if(R_FAILED(ret = svcSendSyncRequest(acHandle)))return ret;

	*enable = cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result ACU_GetProxyPort(u32 *out)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x38,0,0); // 0x380000

	if(R_FAILED(ret = svcSendSyncRequest(acHandle)))return ret;

	*out = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result ACU_GetProxyUserName(char *username)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	cmdbuf[0] = IPC_MakeHeader(0x3A,0,0); // 0x3A0000

	staticbufs[0] = IPC_Desc_StaticBuffer(0x20, 0);
	staticbufs[1] = (u32)username;

	if(R_FAILED(ret = svcSendSyncRequest(acHandle)))return ret;

	return (Result)cmdbuf[1];
}
