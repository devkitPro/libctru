#include <3ds/ipc.h>
#include <3ds/result.h>
#include <3ds/services/cecd.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>

static Handle cecdHandle;
static int cecdRefCount;

Result cecdInit(bool force_user) {
	Result res = -1;

	if (AtomicPostIncrement(&cecdRefCount)) return 0;
	
	if (!force_user) {
		res = srvGetServiceHandle(&cecdHandle, "cecd:s");
	}
	
	if (R_FAILED(res)) {
		res = srvGetServiceHandle(&cecdHandle, "cecd:u");
	}

	if (R_FAILED(res)) AtomicDecrement(&cecdRefCount);
	return res;
}

Result cecdReadMessage(u32 title_id, bool is_outbox, CecMessageId message_id, u8* buf, u32 size) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x03, 4, 4);
	cmdbuf[1] = title_id;
	cmdbuf[2] = (u32)is_outbox;
	cmdbuf[3] = 8; // message id size
	cmdbuf[4] = size;

	cmdbuf[5] = IPC_Desc_Buffer(8, IPC_BUFFER_R);
	cmdbuf[6] = (u32)message_id;
	cmdbuf[7] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[8] = (u32)buf;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdReadMessageWithHMAC(u32 title_id, bool is_outbox, CecMessageId message_id, u8* buf, u32 size, u8* hmac) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x03, 4, 4);
	cmdbuf[1] = title_id;
	cmdbuf[2] = (u32)is_outbox;
	cmdbuf[3] = 8; // message id size
	cmdbuf[4] = size;

	cmdbuf[5] = IPC_Desc_Buffer(8, IPC_BUFFER_R);
	cmdbuf[6] = (u32)message_id;
	cmdbuf[7] = IPC_Desc_Buffer(32, IPC_BUFFER_R);
	cmdbuf[8] = (u32)hmac;
	cmdbuf[9] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[10] = (u32)buf;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdWriteMessage(u32 title_id, bool is_outbox, CecMessageId message_id, u8* buf, u32 size) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x06, 4, 4);
	cmdbuf[1] = title_id;
	cmdbuf[2] = (u32)is_outbox;
	cmdbuf[3] = 8; // message id size
	cmdbuf[4] = size;

	cmdbuf[5] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[6] = (u32)buf;
	cmdbuf[7] = IPC_Desc_Buffer(8, IPC_BUFFER_RW);
	cmdbuf[8] = (u32)message_id;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdWriteMessageWithHMAC(u32 title_id, bool is_outbox, CecMessageId message_id, u8* buf, u32 size, u8* hmac) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x07, 4, 6);
	cmdbuf[1] = title_id;
	cmdbuf[2] = (u32)is_outbox;
	cmdbuf[3] = 8; // message id size
	cmdbuf[4] = size;

	cmdbuf[5] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[6] = (u32)buf;
	cmdbuf[7] = IPC_Desc_Buffer(32, IPC_BUFFER_R);
	cmdbuf[8] = (u32)hmac;
	cmdbuf[9] = IPC_Desc_Buffer(8, IPC_BUFFER_RW);
	cmdbuf[10] = (u32)message_id;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdStart(CecCommand command) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x0B, 1, 0);
	cmdbuf[1] = command;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdStop(CecCommand command) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x0C, 1, 0);
	cmdbuf[1] = command;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdGetState(CecStateAbbrev* state) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xE, 0, 0);
	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];
	*state = cmdbuf[2];

	return res;
}

Result cecdGetCecInfoEventHandle(Handle* handle) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xF, 0, 0);
	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];
	*handle = cmdbuf[3];

	return res;
}

Result cecdGetChangeStateEventHandle(Handle* handle) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x10, 0, 0);
	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];
	*handle = cmdbuf[3];

	return res;
}

Result cecdOpenAndWrite(u32 title_id, CecPath path_type, u8* buf, u32 size) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x11, 4, 4);
	cmdbuf[1] = size;
	cmdbuf[2] = title_id;
	cmdbuf[3] = path_type;
	cmdbuf[4] = 0;

	cmdbuf[5] = IPC_Desc_CurProcessId();
	cmdbuf[6] = 0;
	cmdbuf[7] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[8] = (u32)buf;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdOpenAndRead(u32 title_id, CecPath path_type, u8* buf, u32 size) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x12, 4, 4);
	cmdbuf[1] = size;
	cmdbuf[2] = title_id;
	cmdbuf[3] = path_type;
	cmdbuf[4] = 0;

	cmdbuf[5] = IPC_Desc_CurProcessId();
	cmdbuf[6] = 0;
	cmdbuf[7] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[8] = (u32)buf;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdSprCreate(void) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x40A, 0, 0);

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdSprInitialise(void) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x40B, 0, 0);

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdSprGetSlotsMetadata(CecSlotMetadata* buf, u32 size, u32* slots_total) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x40C, 1, 2);
	cmdbuf[1] = size;

	cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[3] = (u32)buf;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];
	*slots_total = cmdbuf[2];

	return res;
}

Result cecdSprGetSlot(u32 title_id, u8* buf, u32 size) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x40D, 2, 2);
	cmdbuf[1] = title_id;
	cmdbuf[2] = size;

	cmdbuf[3] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[4] = (u32)buf;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdSprSetTitleSent(u32 title_id, bool success) { // set send result
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x40E, 2, 0);
	cmdbuf[1] = title_id;
	cmdbuf[2] = success;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdSprFinaliseSend(void) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x40F, 0, 0);

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdSprStartRecv(void) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x410, 0, 0);

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdSprAddSlotsMetadata(u8* buf, u32 size) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x411, 1, 2);
	cmdbuf[1] = size;

	cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[3] = (u32)buf;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdSprAddSlot(u32 title_id, u8* buf, u32 size) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x412, 3, 2);
	cmdbuf[1] = title_id;
	cmdbuf[2] = 0xFF; // flags
	cmdbuf[3] = size;

	cmdbuf[4] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[5] = (u32)buf;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdSprFinaliseRecv(void) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x413, 0, 0);

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdSprDone(bool success) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x414, 1, 0);
	cmdbuf[1] = success;

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];

	return res;
}

Result cecdGetBossUserid(u64* out) {
	Result res = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x415, 0, 0);

	if (R_FAILED(res = svcSendSyncRequest(cecdHandle))) return res;
	res = (Result)cmdbuf[1];
	*out = (u64)cmdbuf[2] | ((u64)cmdbuf[3] << 32);

	return res;
}

Handle cecdGetServHandle(void) {
	return cecdHandle;
}
