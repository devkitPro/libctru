#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/os.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/news.h>
#include <3ds/ipc.h>

typedef struct {
	bool dataSet;
	bool unread;
	bool enableJPEG;
	u8 unkFlag1;
	u8 unkFlag2;
	u64 processID;
	u8 unkData[24];
	u64 time;
	u16 title[32];
} NotificationHeader;

static Handle newsHandle;
static int newsRefCount;
static bool useNewsS;

Result newsInit(void) {
	Result res;
	if (AtomicPostIncrement(&newsRefCount)) return 0;
	res = srvGetServiceHandle(&newsHandle, "news:u");
	useNewsS = R_FAILED(res);
	if (useNewsS) res = srvGetServiceHandle(&newsHandle, "news:s");
	if (R_FAILED(res)) AtomicDecrement(&newsRefCount);
	return res;
}

void newsExit(void) {
	if (AtomicDecrement(&newsRefCount)) return;
	svcCloseHandle(newsHandle);
}

Result NEWS_AddNotification(const u16* title, u32 titleLength, const u16* message, u32 messageLength, const void* imageData, u32 imageSize, bool jpeg)
{
	NotificationHeader header = { 0 };
	header.dataSet = true;
	header.unread = true;
	header.enableJPEG = jpeg;
	header.processID = 0; // Filled automatically from FS:GetProgramLaunchInfo
	header.time = osGetTime();
	memcpy(header.title, title, (titleLength < 32 ? titleLength + 1 : 32) * sizeof(u16));

	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,3,8); // 0x100C8
	cmdbuf[1] = sizeof(NotificationHeader);
	cmdbuf[2] = (messageLength + 1) * sizeof(u16);
	cmdbuf[3] = imageSize;

	u32 baseIndex = 4;
	if (!useNewsS) {
		cmdbuf[baseIndex] = IPC_Desc_CurProcessHandle();
		cmdbuf[baseIndex + 1] = 0; // Process ID, Filled automatically by the ARM11 kernel.
		baseIndex += 2;
	}

	cmdbuf[baseIndex] = IPC_Desc_Buffer(sizeof(NotificationHeader),IPC_BUFFER_R);
	cmdbuf[baseIndex + 1] = (u32) &header;
	cmdbuf[baseIndex + 2] = IPC_Desc_Buffer((messageLength + 1) * sizeof(u16),IPC_BUFFER_R);
	cmdbuf[baseIndex + 3] = (u32) message;
	cmdbuf[baseIndex + 4] = IPC_Desc_Buffer(imageSize,IPC_BUFFER_R);
	cmdbuf[baseIndex + 5] = (u32) imageData;

	if(R_FAILED(ret = svcSendSyncRequest(newsHandle))) return ret;

	return (Result)cmdbuf[1];
}
