#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/pxidev.h>
#include <3ds/ipc.h>

static Handle pxiDevHandle;
static int pxiDevRefCount;

Result pxiDevInit(void)
{
	Result ret;

	if (AtomicPostIncrement(&pxiDevRefCount)) return 0;

	ret = srvGetServiceHandle(&pxiDevHandle, "pxi:dev");
	if (R_FAILED(ret)) AtomicDecrement(&pxiDevRefCount);

	return ret;
}

void pxiDevExit(void)
{
	if (AtomicDecrement(&pxiDevRefCount)) return;
	svcCloseHandle(pxiDevHandle);
}

Result PXIDEV_SPIMultiWriteRead(PXIDEV_SPIBuffer* header, PXIDEV_SPIBuffer* writeBuffer1, PXIDEV_SPIBuffer* readBuffer1, PXIDEV_SPIBuffer* writeBuffer2, PXIDEV_SPIBuffer* readBuffer2, PXIDEV_SPIBuffer* footer)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xD,26,8); // 0x000D0688
	memcpy(&cmdbuf[1], header->ptr, header->size);
	cmdbuf[3] = header->size;
	cmdbuf[4] = header->transferOption;
	cmdbuf[5] = (u32) (header->waitOperation & 0xFFFFFFFF);
	cmdbuf[6] = (u32) ((header->waitOperation >> 32) & 0xFFFFFFFF);
	cmdbuf[7] = writeBuffer1->size;
	cmdbuf[8] = writeBuffer1->transferOption;
	cmdbuf[9] = (u32) (writeBuffer1->waitOperation & 0xFFFFFFFF);
	cmdbuf[10] = (u32) ((writeBuffer1->waitOperation >> 32) & 0xFFFFFFFF);
	cmdbuf[11] = readBuffer1->size;
	cmdbuf[12] = readBuffer1->transferOption;
	cmdbuf[13] = (u32) (readBuffer1->waitOperation & 0xFFFFFFFF);
	cmdbuf[14] = (u32) ((readBuffer1->waitOperation >> 32) & 0xFFFFFFFF);
	cmdbuf[15] = writeBuffer2->size;
	cmdbuf[16] = writeBuffer2->transferOption;
	cmdbuf[17] = (u32) (writeBuffer2->waitOperation & 0xFFFFFFFF);
	cmdbuf[18] = (u32) ((writeBuffer2->waitOperation >> 32) & 0xFFFFFFFF);
	cmdbuf[19] = readBuffer2->size;
	cmdbuf[20] = readBuffer2->transferOption;
	cmdbuf[21] = (u32) (readBuffer2->waitOperation & 0xFFFFFFFF);
	cmdbuf[22] = (u32) ((readBuffer2->waitOperation >> 32) & 0xFFFFFFFF);
	memcpy(&cmdbuf[23], footer->ptr, footer->size);
	cmdbuf[25] = footer->size;
	cmdbuf[26] = footer->transferOption;
	cmdbuf[27] = IPC_Desc_PXIBuffer(writeBuffer1->size, 0, true);
	cmdbuf[28] = (u32) writeBuffer1->ptr;
	cmdbuf[29] = IPC_Desc_PXIBuffer(writeBuffer2->size, 1, true);
	cmdbuf[30] = (u32) writeBuffer2->ptr;
	cmdbuf[31] = IPC_Desc_PXIBuffer(readBuffer1->size, 2, false);
	cmdbuf[32] = (u32) readBuffer1->ptr;
	cmdbuf[33] = IPC_Desc_PXIBuffer(readBuffer2->size, 3, false);
	cmdbuf[34] = (u32) readBuffer2->ptr;

	if (R_FAILED(ret = svcSendSyncRequest(pxiDevHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result PXIDEV_SPIWriteRead(u32* bytesRead, u64 initialWaitOperation, PXIDEV_SPIBuffer* writeBuffer, PXIDEV_SPIBuffer* readBuffer)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xE,10,4); // 0x000E0284
	cmdbuf[1] = (u32) (initialWaitOperation & 0xFFFFFFFF);
	cmdbuf[2] = (u32) ((initialWaitOperation >> 32) & 0xFFFFFFFF);
	cmdbuf[3] = writeBuffer->size;
	cmdbuf[4] = writeBuffer->transferOption;
	cmdbuf[5] = (u32) (writeBuffer->waitOperation & 0xFFFFFFFF);
	cmdbuf[6] = (u32) ((writeBuffer->waitOperation >> 32) & 0xFFFFFFFF);
	cmdbuf[7] = readBuffer->size;
	cmdbuf[8] = readBuffer->transferOption;
	cmdbuf[9] = (u32) (readBuffer->waitOperation & 0xFFFFFFFF);
	cmdbuf[10] = (u32) ((readBuffer->waitOperation >> 32) & 0xFFFFFFFF);
	cmdbuf[11] = IPC_Desc_PXIBuffer(writeBuffer->size, 0, true);
	cmdbuf[12] = (u32) writeBuffer->ptr;
	cmdbuf[13] = IPC_Desc_PXIBuffer(readBuffer->size, 1, false);
	cmdbuf[14] = (u32) readBuffer->ptr;

	if (R_FAILED(ret = svcSendSyncRequest(pxiDevHandle))) return ret;

	if (bytesRead) *bytesRead = cmdbuf[2];

	return (Result)cmdbuf[1];
}
