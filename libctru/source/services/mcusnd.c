#include <3ds/services/mcu_common.h>
#include <3ds/synchronization.h>
#include <3ds/result.h>
#include <3ds/types.h>
#include <3ds/ipc.h>
#include <3ds/srv.h>

static Handle mcuSndHandle;
static int mcuSndRefCount;

Result mcuSndInit(void)
{
	if (AtomicPostIncrement(&mcuSndRefCount)) return 0;
	Result res = srvGetServiceHandle(&mcuSndHandle, "mcu::SND");
	if (R_FAILED(res)) AtomicDecrement(&mcuSndRefCount);
	return res;
}

void mcuSndExit(void)
{
	if (AtomicDecrement(&mcuSndRefCount)) return;
	svcCloseHandle(mcuSndHandle);
}

Handle *mcuSndGetSessionHandle(void)
{
	return &mcuSndHandle;
}

Result MCUSND_GetVolumeSliderLevel(u8 *level)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0001, 0, 0); // 0x00010000

	Result res = svcSendSyncRequest(mcuSndHandle);
	if (R_FAILED(res)) return res;

	*level = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MCUSND_WriteReg25h(u8 value)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0002, 1, 0); // 0x00020040
	cmdbuf[1] = value;
	
	Result res = svcSendSyncRequest(mcuSndHandle);
	if (R_FAILED(res)) return res;
	return (Result)cmdbuf[1];
}

Result MCUSND_ReadReg25h(u8 *out_value)
{
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0x0003, 0, 0); // 0x00030000
	
	Result res = svcSendSyncRequest(mcuSndHandle);
	if (R_FAILED(res)) return res;
	
	*out_value = (u8)(cmdbuf[2] & 0xFF);
	
	return (Result)cmdbuf[1];
}