#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/ipc.h>
#include <3ds/services/cdcchk.h>

static Handle cdcChkHandle;
static int cdcChkRefCount;

Result cdcChkInit(void)
{
	if (AtomicPostIncrement(&cdcChkRefCount))
		return 0;

	Result res = srvGetServiceHandle(&cdcChkHandle, "cdc:CHK");
	if (R_FAILED(res))
		AtomicDecrement(&cdcChkRefCount);
	return res;
}

void cdcChkExit(void)
{
	if (AtomicDecrement(&cdcChkRefCount))
		return;
	svcCloseHandle(cdcChkHandle);
}

Handle *cdcChkGetSessionHandle(void)
{
	return &cdcChkHandle;
}

Result CDCCHK_ReadRegisters1(u8 pageId, u8 initialRegAddr, void *outData, size_t size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 staticbufBackup[2];
	memcpy(staticbufBackup, staticbufs, 8);

	staticbufs[0] = IPC_Desc_StaticBuffer(size, 0);
	staticbufs[1] = (u32)outData;

	cmdbuf[0] = IPC_MakeHeader(1, 3, 0); // 0x100C0
	cmdbuf[1] = pageId;
	cmdbuf[2] = initialRegAddr;
	cmdbuf[3] = size;

	ret = svcSendSyncRequest(cdcChkHandle);

	memcpy(staticbufs, staticbufBackup, 8);

	return R_SUCCEEDED(ret) ? cmdbuf[1] : ret;
}

Result CDCCHK_ReadRegisters2(u8 pageId, u8 initialRegAddr, void *outData, size_t size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 staticbufBackup[2];
	memcpy(staticbufBackup, staticbufs, 8);

	staticbufs[0] = IPC_Desc_StaticBuffer(size, 0);
	staticbufs[1] = (u32)outData;

	cmdbuf[0] = IPC_MakeHeader(2, 3, 0); // 0x200C0
	cmdbuf[1] = pageId;
	cmdbuf[2] = initialRegAddr;
	cmdbuf[3] = size;

	ret = svcSendSyncRequest(cdcChkHandle);

	memcpy(staticbufs, staticbufBackup, 8);

	return R_SUCCEEDED(ret) ? cmdbuf[1] : ret;
}

Result CDCCHK_WriteRegisters1(u8 pageId, u8 initialRegAddr, const void *data, size_t size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(3, 3, 2); // 0x300C2
	cmdbuf[1] = pageId;
	cmdbuf[2] = initialRegAddr;
	cmdbuf[3] = size;
	cmdbuf[4] = IPC_Desc_StaticBuffer(size, 0);
	cmdbuf[5] = (u32)data;

	ret = svcSendSyncRequest(cdcChkHandle);

	return R_SUCCEEDED(ret) ? cmdbuf[1] : ret;
}

Result CDCCHK_WriteRegisters2(u8 pageId, u8 initialRegAddr, const void *data, size_t size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(4, 3, 2); // 0x400C2
	cmdbuf[1] = pageId;
	cmdbuf[2] = initialRegAddr;
	cmdbuf[3] = size;
	cmdbuf[4] = IPC_Desc_StaticBuffer(size, 0);
	cmdbuf[5] = (u32)data;

	ret = svcSendSyncRequest(cdcChkHandle);

	return R_SUCCEEDED(ret) ? cmdbuf[1] : ret;
}

Result CDCCHK_ReadNtrPmicRegister(u8 *outData, u8 regAddr)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(5, 1, 0); // 0x50040
	cmdbuf[1] = regAddr;

	ret = svcSendSyncRequest(cdcChkHandle);
	if (R_SUCCEEDED(ret))
	{
		*outData = (u8)cmdbuf[2];
		ret = cmdbuf[1];
	}

	return ret;
}

Result CDCCHK_WriteNtrPmicRegister(u8 regAddr, u8 data)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(6, 2, 0); // 0x60080
	cmdbuf[1] = regAddr;
	cmdbuf[2] = data;

	ret = svcSendSyncRequest(cdcChkHandle);

	return R_SUCCEEDED(ret) ? cmdbuf[1] : ret;
}

Result CDCCHK_SetI2sVolume(CodecI2sLine i2sLine, s8 volume)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(7, 2, 0); // 0x70080
	cmdbuf[1] = (u32)i2sLine;
	cmdbuf[2] = volume;

	ret = svcSendSyncRequest(cdcChkHandle);

	return R_SUCCEEDED(ret) ? cmdbuf[1] : ret;
}
