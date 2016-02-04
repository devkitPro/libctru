#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/nfc.h>
#include <3ds/ipc.h>

static Handle nfcHandle;
static int nfcRefCount;

Result nfcInit(void)
{
	Result ret=0;

	if (AtomicPostIncrement(&nfcRefCount)) return 0;

	ret = srvGetServiceHandle(&nfcHandle, "nfc:u");
	if (R_SUCCEEDED(ret))
	{
		ret = nfc_Initialize(0x02);
		if (R_FAILED(ret)) svcCloseHandle(nfcHandle);
	}
	if (R_FAILED(ret)) AtomicDecrement(&nfcRefCount);

	return ret;
}

void nfcExit(void)
{
	if (AtomicDecrement(&nfcRefCount)) return;
	svcCloseHandle(nfcHandle);
}

Handle nfcGetSessionHandle(void)
{
	return nfcHandle;
}

Result nfc_Initialize(u8 type)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1,1,0); // 0x10040
	cmdbuf[1]=type;
	
	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	
	ret = (Result)cmdbuf[1];

	return ret;
}

Result nfc_Shutdown(u8 type)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x2,1,0); // 0x20040
	cmdbuf[1]=type;
	
	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	
	ret = (Result)cmdbuf[1];

	return ret;
}

Result nfc_StartCommunication()
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x3,0,0); // 0x30000
	
	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	
	ret = (Result)cmdbuf[1];

	return ret;
}

Result nfc_StopCommunication()
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x4,0,0); // 0x40000
	
	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	
	ret = (Result)cmdbuf[1];

	return ret;
}

Result nfc_StartTagScanning(u16 unknown)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x5,1,0); // 0x50040
	cmdbuf[1]=unknown;
	
	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	
	ret = (Result)cmdbuf[1];

	return ret;
}

Result nfc_StopTagScanning()
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x6,0,0); // 0x60000
	
	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	
	ret = (Result)cmdbuf[1];

	return ret;
}

Result nfc_LoadAmiiboData()
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x7,0,0); // 0x70000
	
	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	
	ret = (Result)cmdbuf[1];

	return ret;
}

Result nfc_ResetTagScanState()
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x8,0,0); // 0x80000
	
	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	
	ret = (Result)cmdbuf[1];

	return ret;
}

Result nfc_GetTagState(u8 *state)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xD,0,0); // 0xD0000
	
	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	
	ret = (Result)cmdbuf[1];
	*state = cmdbuf[2];

	return ret;
}

