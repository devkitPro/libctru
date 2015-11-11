#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/cfgu.h>
#include <3ds/ipc.h>

static Handle cfguHandle;
static int cfguRefCount;

Result cfguInit(void)
{
	Result ret;

	if (AtomicPostIncrement(&cfguRefCount)) return 0;

	// cfg:i has the most commands, then cfg:s, then cfg:u
	ret = srvGetServiceHandle(&cfguHandle, "cfg:i");
	if(R_FAILED(ret)) ret = srvGetServiceHandle(&cfguHandle, "cfg:s");
	if(R_FAILED(ret)) ret = srvGetServiceHandle(&cfguHandle, "cfg:u");
	if(R_FAILED(ret)) AtomicDecrement(&cfguRefCount);

	return ret;
}

void cfguExit(void)
{
	if (AtomicDecrement(&cfguRefCount)) return;
	svcCloseHandle(cfguHandle);
}

Result CFGU_SecureInfoGetRegion(u8* region)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,0,0); // 0x20000

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	*region = (u8)cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result CFGU_GenHashConsoleUnique(u32 appIDSalt, u64* hash)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3,1,0); // 0x30040
	cmdbuf[1] = appIDSalt;

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	*hash = (u64)cmdbuf[2];
	*hash |= ((u64)cmdbuf[3])<<32;

	return (Result)cmdbuf[1];
}

Result CFGU_GetRegionCanadaUSA(u8* value)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4,0,0); // 0x40000

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	*value = (u8)cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result CFGU_GetSystemModel(u8* model)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5,0,0); // 0x50000

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	*model = (u8)cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result CFGU_GetModelNintendo2DS(u8* value)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6,0,0); // 0x60000

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	*value = (u8)cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result CFGU_GetCountryCodeString(u16 code, u16* string)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x9,1,0); // 0x90040
	cmdbuf[1] = (u32)code;

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	*string = (u16)cmdbuf[2] & 0xFFFF;

	return (Result)cmdbuf[1];
}

Result CFGU_GetCountryCodeID(u16 string, u16* code)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xA,1,0); // 0xA0040
	cmdbuf[1] = (u32)string;

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	*code = (u16)cmdbuf[2] & 0xFFFF;

	return (Result)cmdbuf[1];
}

// See here for block IDs:
// http://3dbrew.org/wiki/Config_Savegame#Configuration_blocks
Result CFGU_GetConfigInfoBlk2(u32 size, u32 blkID, u8* outData)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,2,2); // 0x10082
	cmdbuf[1] = size;
	cmdbuf[2] = blkID;
	cmdbuf[3] = IPC_Desc_Buffer(size,IPC_BUFFER_W);
	cmdbuf[4] = (u32)outData;

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result CFGU_GetSystemLanguage(u8* language)
{
	return CFGU_GetConfigInfoBlk2(1, 0xA0002, language);
}
