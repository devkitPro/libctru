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

Result CFGU_IsNFCSupported(bool* isSupported)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xB,0,0); // 0x000B0000

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	*isSupported = cmdbuf[2] & 0xFF;

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

Result CFG_GetConfigInfoBlk4(u32 size, u32 blkID, u8* outData)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x401,2,2); // 0x4010082
	cmdbuf[1] = size;
	cmdbuf[2] = blkID;
	cmdbuf[3] = IPC_Desc_Buffer(size,IPC_BUFFER_W);
	cmdbuf[4] = (u32)outData;

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result CFG_GetConfigInfoBlk8(u32 size, u32 blkID, u8* outData)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x801,2,2); // 0x8010082
	cmdbuf[1] = size;
	cmdbuf[2] = blkID;
	cmdbuf[3] = IPC_Desc_Buffer(size,IPC_BUFFER_W);
	cmdbuf[4] = (u32)outData;

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result CFG_SetConfigInfoBlk4(u32 size, u32 blkID, u8* inData)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x402,2,2); // 0x4020082
	cmdbuf[1] = blkID;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_Buffer(size,IPC_BUFFER_R);
	cmdbuf[4] = (u32)inData;

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result CFG_SetConfigInfoBlk8(u32 size, u32 blkID, u8* inData)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x802,2,2); // 0x8020082
	cmdbuf[1] = blkID;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_Buffer(size,IPC_BUFFER_R);
	cmdbuf[4] = (u32)inData;

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result CFG_UpdateConfigSavegame(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x803,0,0); // 0x8030000

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result CFGU_GetSystemLanguage(u8* language)
{
	return CFGU_GetConfigInfoBlk2(1, 0xA0002, language);
}

Result CFGI_RestoreLocalFriendCodeSeed(void) 
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80D, 0, 0); // 0x80D0000

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result CFGI_RestoreSecureInfo(void) 
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x812,0,0); // 0x8120000

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result CFGI_DeleteConfigSavefile(void) 
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x805,0,0); // 0x8050000

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result CFGI_FormatConfig(void) 
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x806,0,0); // 0x8060000

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result CFGI_ClearParentalControls(void) 
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x40F,0,0); // 0x40F0000

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result CFGI_VerifySigLocalFriendCodeSeed(void) 
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80E,0,0); // 0x80E0000

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result CFGI_VerifySigSecureInfo(void) 
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x813,0,0); // 0x8130000

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result CFGI_SecureInfoGetSerialNumber(u8 *serial)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x408,1,2); // 0x4080042
	cmdbuf[1] = 0xF;
	cmdbuf[2] = IPC_Desc_Buffer(0xF, IPC_BUFFER_W);
	cmdbuf[3] = (u32)serial;

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result CFGI_GetLocalFriendCodeSeedData(u8 *data)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x404,1,2); // 0x4040042
	cmdbuf[1] = 0x110;
	cmdbuf[2] = IPC_Desc_Buffer(0x110, IPC_BUFFER_W);
	cmdbuf[3] = (u32)data;

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result CFGI_GetLocalFriendCodeSeed(u64* seed)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x405,0,0); // 0x4050000

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	*seed = (u64)cmdbuf[2] | (u64)cmdbuf[3] << 32;

	return (Result)cmdbuf[1];
}

Result CFGI_GetSecureInfoData(u8 *data)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x814,1,2); // 0x8140042
	cmdbuf[1] = 0x11;
	cmdbuf[2] = IPC_Desc_Buffer(0x11, IPC_BUFFER_W);
	cmdbuf[3] = (u32)data;

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result CFGI_GetSecureInfoSignature(u8 *data)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x815,1,2); // 0x8150042
	cmdbuf[1] = 0x100;
	cmdbuf[2] = IPC_Desc_Buffer(0x100, IPC_BUFFER_W);
	cmdbuf[3] = (u32)data;

	if(R_FAILED(ret = svcSendSyncRequest(cfguHandle)))return ret;

	return (Result)cmdbuf[1];
}
