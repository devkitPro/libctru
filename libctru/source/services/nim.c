#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/nim.h>
#include <3ds/ipc.h>
#include <3ds/util/utf.h>

static Handle nimsHandle;
static int nimsRefCount;

static Result NIMS_GetInitializeResult(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3f, 0, 0); // 0x003f0000

	if (R_FAILED(ret = svcSendSyncRequest(nimsHandle))) return ret;

	return (Result)cmdbuf[1];
}

static Result NIMS_RegisterSelf(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3c, 0, 2); // 0x003c0002
	cmdbuf[1] = IPC_Desc_CurProcessHandle();
	cmdbuf[2] = IPC_Desc_MoveHandles(1);

	if (R_FAILED(ret = svcSendSyncRequest(nimsHandle))) return ret;

	return (Result)cmdbuf[1];
}

static Result NIMS_SetTIN(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	static char attr[] = "TIN";
	static char TIN[] = "56789"; // Obtained from eShop

	cmdbuf[0] = IPC_MakeHeader(0xb, 2, 4); // 0x000b0084
	cmdbuf[1] = sizeof(attr);
	cmdbuf[2] = sizeof(TIN);
	cmdbuf[3] = IPC_Desc_StaticBuffer(cmdbuf[1], 0);
	cmdbuf[4] = (u32)attr;
	cmdbuf[5] = IPC_Desc_StaticBuffer(cmdbuf[2], 1);
	cmdbuf[6] = (u32)TIN;

	if (R_FAILED(ret = svcSendSyncRequest(nimsHandle))) return ret;

	return (Result)cmdbuf[1];
}

static Result NIMS_ConnectNoTicketDownload(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	static u8 buffer[0x20000];

	cmdbuf[0] = IPC_MakeHeader(0x57, 2, 2); // 0x00570082
	cmdbuf[1] = (u32)buffer; // What the fuck?
	cmdbuf[2] = sizeof(buffer);
	cmdbuf[3] = IPC_Desc_Buffer(sizeof(buffer), IPC_BUFFER_W);
	cmdbuf[4] = (u32)buffer;

	if (R_FAILED(ret = svcSendSyncRequest(nimsHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result nimsInit(void)
{
	Result res;
	if (AtomicPostIncrement(&nimsRefCount)) return 0;
	res = srvGetServiceHandle(&nimsHandle, "nim:s");
	if (R_FAILED(res)) AtomicDecrement(&nimsRefCount);

	if (R_FAILED(res = NIMS_GetInitializeResult())) {
		nimsExit();
		return res;
	}

	if (R_FAILED(res = NIMS_RegisterSelf())) {
		nimsExit();
		return res;
	}

	if (R_FAILED(res = NIMS_SetTIN())) {
		nimsExit();
		return res;
	}

	if (R_FAILED(res = NIMS_ConnectNoTicketDownload())) {
		nimsExit();
		return res;
	}

	return res;
}

void nimsExit(void)
{
	if (AtomicDecrement(&nimsRefCount)) return;
	svcCloseHandle(nimsHandle);
}

Handle *nimsGetSessionHandle(void)
{
	return &nimsHandle;
}

Result NIMS_WantUpdate(bool *want_update)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0]  = IPC_MakeHeader(0xa, 0, 0); //0x000a0000

	if (R_FAILED(ret = svcSendSyncRequest(nimsHandle))) return ret;
	if (want_update) *want_update = cmdbuf[2] & 0xFF;

	return cmdbuf[1];
}

void NIMS_MakeTitleConfig(NIM_TitleConfig *cfg, u64 titleId, u32 version, u8 ratingAge, FS_MediaType mediaType)
{
	memset(cfg, 0, sizeof(*cfg));

	cfg->titleId = titleId;
	cfg->version = version;
	cfg->ratingAge = ratingAge;
	cfg->mediaType = mediaType;
}

Result NIMS_RegisterTask(const NIM_TitleConfig *cfg, const char *name, const char *maker)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	static uint16_t name16[0x92], maker16[0x4a];
	ssize_t units;

	units = utf8_to_utf16(name16, (const uint8_t *)name, sizeof(name16)/sizeof(*name16));
	if (units < 0 || units > sizeof(name16)/sizeof(*name16)) return -2;
	units = utf8_to_utf16(maker16, (const uint8_t *)maker, sizeof(maker16)/sizeof(*maker16));
	if (units < 0 || units > sizeof(maker16)/sizeof(*maker16)) return -2;

	cmdbuf[0]  = IPC_MakeHeader(0x55, 9, 6); //0x00550246
	memcpy(&cmdbuf[1], cfg, sizeof(*cfg));
	// unused: cmdbuf[7], cmdbuf[8]
	cmdbuf[9]  = 0; // always 0? At least it is for eShop
	cmdbuf[10] = IPC_Desc_CurProcessHandle();
	// unused: cmdbuf[11]
	cmdbuf[12] = IPC_Desc_StaticBuffer(sizeof(name16), 1);
	cmdbuf[13] = (unsigned int)name16;
	cmdbuf[14] = IPC_Desc_StaticBuffer(sizeof(maker16), 5);
	cmdbuf[15] = (unsigned int)maker16;

	if (R_FAILED(ret = svcSendSyncRequest(nimsHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result NIMS_IsTaskRegistered(u64 titleId, bool *registered)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6, 2, 0); //0x00060080
	cmdbuf[1] = titleId & 0xFFFFFFFF;
	cmdbuf[2] = (u32) ((titleId >> 32) & 0xFFFFFFFF);

	if (R_FAILED(ret = svcSendSyncRequest(nimsHandle))) return ret;
	if (registered) *registered = cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result NIMS_UnregisterTask(u64 titleId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5, 2, 2); //0x00050082
	cmdbuf[1] = titleId & 0xFFFFFFFF;
	cmdbuf[2] = (u32) ((titleId >> 32) & 0xFFFFFFFF);
	cmdbuf[3] = IPC_Desc_CurProcessHandle();

	if (R_FAILED(ret = svcSendSyncRequest(nimsHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result NIMS_StartDownload(const NIM_TitleConfig *cfg, NIM_InstallationMode mode)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0]  = IPC_MakeHeader(0x42, 9, 0); //0x00420240
	memcpy(&cmdbuf[1], cfg, sizeof(*cfg));
	// unused: cmdbuf[7], cmdbuf[8]
	cmdbuf[9]  = mode;

	if (R_FAILED(ret = svcSendSyncRequest(nimsHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result NIMS_StartDownloadSimple(const NIM_TitleConfig *cfg)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0]  = IPC_MakeHeader(0x1, 8, 0); //0x00010200
	memcpy(&cmdbuf[1], cfg, sizeof(*cfg));

	if (R_FAILED(ret = svcSendSyncRequest(nimsHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result NIMS_GetProgress(NIM_TitleProgress *tp)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0]  = IPC_MakeHeader(0x3, 0, 0); //0x00030000

	if (R_FAILED(ret = svcSendSyncRequest(nimsHandle)))
		return ret;

	memcpy(tp, &cmdbuf[2], sizeof(*tp));

	return (Result)cmdbuf[1];
}

Result NIMS_CancelDownload(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0]  = IPC_MakeHeader(0x2, 0, 0); //0x00020000

	if (R_FAILED(ret = svcSendSyncRequest(nimsHandle))) return ret;

	return (Result)cmdbuf[1];
}
