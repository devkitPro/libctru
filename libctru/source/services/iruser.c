/**
Copy of the IR:USER API from ctru-rs
*/

#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/iruser.h>
#include <3ds/ipc.h>

// Misc constants
const size_t SHARED_MEM_INFO_SECTIONS_SIZE = 0x30;
const size_t SHARED_MEM_RECV_BUFFER_OFFSET = 0x20;
const size_t PAGE_SIZE = 0x1000;
const u32 IR_BITRATE = 4;
const u8 CIRCLE_PAD_PRO_INPUT_RESPONSE_PACKET_ID = 0x10;
const u8 PACKET_INFO_SIZE = 8;

static Handle iruserHandle;
static Handle iruserSharedMemHandle;
static u32 *iruserSharedMem;
static u32 iruserSharedMemSize;
static int iruserRefCount;
static u32 iruserRecvBufferSize;
static u32 iruserRecvPacketCount;
static IRUSER_PacketInfo* iruserRecvPacketInfoBuffer;
static u8* iruserRecvPacketDataBuffer;
static u32 iruserRecvPacketDataBufferSize;

static const u8 CRC_TABLE[256] = {
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
    0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
    0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
    0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
    0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
    0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
    0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
    0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
    0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
    0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
    0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
    0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
    0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
    0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
    0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
    0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
    0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};

u8 crc8ccitt(const void * data, size_t size) {
	u8 val = 0;

	u8 * pos = (u8 *) data;
	u8 * end = pos + size;

	while (pos < end) {
		val = CRC_TABLE[val ^ *pos];
		pos++;
	}

	return val;
}

Result iruserInit(u32 *sharedmem_addr, u32 sharedmem_size, size_t buffer_size, size_t packet_count) {
    if(AtomicPostIncrement(&iruserRefCount)) return 0;
    Result ret = srvGetServiceHandle(&iruserHandle, "ir:USER");
    if (R_FAILED(ret)) goto cleanup0;

    // Calculate the shared memory size.
    // Shared memory length must be a multiple of the page size.
    iruserSharedMemSize = sharedmem_size;
    iruserSharedMem = sharedmem_addr;

    ret = svcCreateMemoryBlock(&iruserSharedMemHandle, (u32)iruserSharedMem, iruserSharedMemSize, MEMPERM_READ, MEMPERM_READWRITE);
    if (R_FAILED(ret)) goto cleanup1;

    ret = IRUSER_InitializeIrNopShared(buffer_size, packet_count, buffer_size, packet_count, IR_BITRATE);
    if (R_FAILED(ret)) goto cleanup2;

    iruserRecvBufferSize = buffer_size;
    iruserRecvPacketCount = packet_count;
    iruserRecvPacketInfoBuffer = (IRUSER_PacketInfo*)((u8*)iruserSharedMem + SHARED_MEM_RECV_BUFFER_OFFSET);
    iruserRecvPacketDataBuffer = (u8*)iruserRecvPacketInfoBuffer + iruserRecvPacketCount * PACKET_INFO_SIZE;
    iruserRecvPacketDataBufferSize = (u32)((u8*)iruserSharedMem - iruserRecvPacketDataBuffer);

    return ret;

    cleanup2:
    IRUSER_FinalizeIrNop();

    cleanup1:
    svcCloseHandle(iruserSharedMemHandle);

    cleanup0:
    return ret;
}

void iruserExit(void) {
    if(AtomicDecrement(&iruserRefCount)) return;

	IRUSER_FinalizeIrNop();
	svcCloseHandle(iruserHandle);
	svcCloseHandle(iruserSharedMemHandle);

	iruserHandle = 0;
	iruserSharedMemHandle = 0;
}

Result IRUSER_InitializeIrNop(size_t recv_buffer_size, size_t recv_packet_count, size_t send_buffer_size, size_t send_packet_count, u32 bitrate) {
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1, 6, 2); // 0x00010182
	cmdbuf[1] = iruserSharedMemSize;
	cmdbuf[2] = recv_buffer_size;
	cmdbuf[3] = recv_packet_count;
	cmdbuf[4] = send_buffer_size;
	cmdbuf[5] = send_packet_count;
	cmdbuf[6] = bitrate;
	cmdbuf[7] = IPC_Desc_SharedHandles(1);
	cmdbuf[8] = iruserSharedMemHandle;

	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result IRUSER_FinalizeIrNop() {
    Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2, 0, 0); // 0x00020000

	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];

	return ret;
};

Result IRUSER_ClearReceiveBuffer() {
    Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3, 0, 0); // 0x00030000

	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];

	return ret;
};

Result IRUSER_ClearSendBuffer() {
    Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4, 0, 0); // 0x00040000

	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];

	return ret;
};

Result IRUSER_WaitConnection(u8 target_id, u64 timeout) {
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5, 3, 0); // 0x000600C0
	cmdbuf[1] = target_id;
	cmdbuf[2] = timeout & 0xFFFFFFFF;
	cmdbuf[3] = (timeout >> 32) & 0xFFFFFFFF;

	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result IRUSER_RequireConnection(u8 device_id) {
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6, 1, 0); // 0x00060040
	cmdbuf[1] = device_id;

	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result IRUSER_AutoConnection() {
    return MAKERESULT(RL_INFO, RS_NOTSUPPORTED, RM_IR, RD_NOT_IMPLEMENTED);
}

Result IRUSER_AnyConnection() {
    Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x8, 0, 0); // 0x00080000

	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];

	return ret;
};

Result IRUSER_Disconnect() {
    Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x9, 0, 0); // 0x00090000

	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result IRUSER_GetReceiveEvent(Handle* eventhandle) {
    Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xA, 0, 0); // 0x000A0000

	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];
	*eventhandle = cmdbuf[3];

	return ret;
}

Result IRUSER_GetSendEvent(Handle* eventhandle) {
    Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xB, 0, 0); // 0x000B0000

	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];
	*eventhandle = cmdbuf[3];

	return ret;
}

Result IRUSER_GetConnectionStatusEvent(Handle* eventhandle) {
    Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xC, 0, 0); // 0x000C0000

	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];
	*eventhandle = cmdbuf[3];

	return ret;
}

Result IRUSER_SendIrNop(u32 size, u8* inbufptr) {
    Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xD, 1, 2); // 0x000D0042
	cmdbuf[1] = size;
	cmdbuf[2] = (size << 14) | 2;
	cmdbuf[3] = (u32)inbufptr;

	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result IRUSER_SendIrNopLarge(u32 size, u8* inbufptr) {
    Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xE, 1, 2); // 0x000E0042
	cmdbuf[1] = size;
	cmdbuf[2] = (size << 8) | 10;
	cmdbuf[3] = (u32)inbufptr;

	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result IRUSER_ReceiveIrNop() {
    return MAKERESULT(RL_INFO, RS_NOTSUPPORTED, RM_IR, RD_NOT_IMPLEMENTED);
}

Result IRUSER_ReceiveIrNopLarge() {
    return MAKERESULT(RL_INFO, RS_NOTSUPPORTED, RM_IR, RD_NOT_IMPLEMENTED);
}

Result IRUSER_GetLatestReceiveErrorResult(u32* result) {
    Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
   
	cmdbuf[0] = IPC_MakeHeader(0x11, 0, 0); // 0x00110000
   
	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];
	*result = (IRUSER_ConnectionStatus)cmdbuf[2];
   
	return ret;
}

Result IRUSER_GetLatestSendErrorResult(u32* result) {
    Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
   
	cmdbuf[0] = IPC_MakeHeader(0x12, 0, 0); // 0x00120000
   
	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];
	*result = (IRUSER_ConnectionStatus)cmdbuf[2];
   
	return ret;
}

Result IRUSER_GetConnectionStatus(IRUSER_ConnectionStatus* status) {
   	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
   
	cmdbuf[0] = IPC_MakeHeader(0x13, 0, 0); // 0x00130000
   
	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];
	*status = (IRUSER_ConnectionStatus)cmdbuf[2];
   
	return ret;
}

Result IRUSER_GetTryingToConnectStatus(IRUSER_TryingToConnectStatus* status) {
    Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
   
	cmdbuf[0] = IPC_MakeHeader(0x14, 0, 0); // 0x00140000
   
	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];
	*status = cmdbuf[2] & 0xFF; // value is a u8
   
	return ret;
}

Result IRUSER_GetReceiveSizeFreeAndUsed() {
    return MAKERESULT(RL_INFO, RS_NOTSUPPORTED, RM_IR, RD_NOT_IMPLEMENTED);
}

Result IRUSER_GetSendSizeFreeAndUsed() {
    return MAKERESULT(RL_INFO, RS_NOTSUPPORTED, RM_IR, RD_NOT_IMPLEMENTED);
}

Result IRUSER_GetConnectionRole(IRUSER_ConnectionRole* role) {
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
   
	cmdbuf[0] = IPC_MakeHeader(0x17, 0, 0); // 0x00170000
   
	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];
	*role = (IRUSER_ConnectionRole)cmdbuf[2];
   
	return ret;
}

Result IRUSER_InitializeIrNopShared(size_t recv_buffer_size, size_t recv_packet_count, size_t send_buffer_size, size_t send_packet_count, u32 bitrate) {
    Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x18, 6, 2); // 0x00180182
	cmdbuf[1] = iruserSharedMemSize;
	cmdbuf[2] = recv_buffer_size;
	cmdbuf[3] = recv_packet_count;
	cmdbuf[4] = send_buffer_size;
	cmdbuf[5] = send_packet_count;
	cmdbuf[6] = bitrate;
	cmdbuf[7] = IPC_Desc_SharedHandles(1);
	cmdbuf[8] = iruserSharedMemHandle;

	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result IRUSER_ReleaseReceivedData(u32 packet_count) {
    Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x19, 1, 0); // 0x00190040
	cmdbuf[1] = packet_count;

	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result IRUSER_SetOwnMachineId(u8 id) {
    Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1A, 1, 0); // 0x001A0040
	cmdbuf[1] = id;

	if(R_FAILED(ret = svcSendSyncRequest(iruserHandle)))return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

/// This will let you directly read the ir:USER shared memory via a callback.
void iruserProcessSharedMemory(void(*process_fn)(u8*)) {
    process_fn((u8*)iruserSharedMem);
}

/// Read and parse the ir:USER service status data from shared memory.
IRUSER_StatusInfo iruserGetStatusInfo() {
    void* shared_mem = iruserSharedMem;
    IRUSER_StatusInfo status_info;
    // copy over data
    memcpy(&status_info, shared_mem, sizeof(status_info));

    return status_info;
}

Result iruserGetCirclePadProState(IRUSER_Packet* packet, circlePadProInputResponse* response) {
    if (!packet) goto failure;
    if (!packet->payload) goto failure;
    if (packet->payload_length != 6) goto failure;
    if (packet->payload[0] != CIRCLE_PAD_PRO_INPUT_RESPONSE_PACKET_ID) goto failure;
    response->cstick.csPos.dx = (u16)(packet->payload[1] | ((packet->payload[2] & 0x0F) << 8));
    response->cstick.csPos.dy = (u16)(((packet->payload[2] & 0xF0) >> 4) | ((packet->payload[3]) << 4));
    response->status_raw = packet->payload[4];
    response->unknown_field = packet->payload[5];
    return 0;
    failure: 
    memset(response, 0, sizeof(circlePadProInputResponse));
    return -1;
}

Result iruserCirclePadProCStickRead(circlePosition *pos) {
    // Result ret = 0;
    u32 n = 0;
    IRUSER_Packet* packet = iruserGetPackets(&n);
    if (n == 0) return -1;
    if (packet[n-1].payload_length != 6) return RS_INVALIDSTATE;
    if (packet[n-1].payload[0] != CIRCLE_PAD_PRO_INPUT_RESPONSE_PACKET_ID) return RS_INVALIDSTATE;
    pos->dx = (s16)(packet[n-1].payload[1] | ((packet[n-1].payload[2] & 0x0F) << 8));
    pos->dy = (s16)(((packet[n-1].payload[2] & 0xF0) >> 4) | ((packet[n-1].payload[3]) << 4));

    free(packet[n-1].payload);
    free(packet);
    IRUSER_ReleaseReceivedData(n);
    return 0;
}

// since data buffer is a ring buffer, this helper macro makes it easier to access the data
#ifndef IRUSER_PACKET_DATA
#define IRUSER_PACKET_DATA(i) (iruserRecvPacketDataBuffer + (inf.offset + i) % iruserRecvPacketDataBufferSize)

static bool iruserParsePacket(size_t index, IRUSER_Packet* packet) {
    if (packet == NULL) return false;
    IRUSER_PacketInfo inf = iruserRecvPacketInfoBuffer[index % iruserRecvPacketCount];
    packet->magic_number = *IRUSER_PACKET_DATA(0);
    packet->destination_network_id = *IRUSER_PACKET_DATA(1);
    bool large = *IRUSER_PACKET_DATA(2) & 0x40;
    u32 payload_offset = 0;
    if (large) {
        packet->payload_length = (*IRUSER_PACKET_DATA(2) << 8) + *IRUSER_PACKET_DATA(3);
        packet->payload = malloc(packet->payload_length);
        payload_offset = 4;
    } else {
        packet->payload_length = *IRUSER_PACKET_DATA(2);
        packet->payload = malloc(packet->payload_length);
        payload_offset = 3;
    }
    for (size_t i = 0; i < packet->payload_length; i++) {
        packet->payload[i] = *IRUSER_PACKET_DATA(i + payload_offset);
    }
    packet->checksum = *IRUSER_PACKET_DATA(packet->payload_length + payload_offset);
    // check the checksum
    u8 checksum = crc8ccitt(IRUSER_PACKET_DATA(0), packet->payload_length + payload_offset);
    if (packet->checksum != checksum) { // bad data
        free(packet->payload);
        packet->payload = NULL;
        return false;
    }
    return true;
}

#endif

#undef IRUSER_PACKET_DATA

/// Read and parse the current packets received from the IR device.
IRUSER_Packet* iruserGetPackets(u32* n) {
    void* shared_mem = iruserSharedMem;

    // Find where the packets are, and how many
    IRUSER_BufferInfo buffer_info;
    memcpy(&buffer_info, (u8*)shared_mem + 0x10, sizeof(buffer_info));
    
    if (n) *n = buffer_info.valid_packet_count;
    if (buffer_info.valid_packet_count == 0) {return NULL;}
    

    IRUSER_Packet* packets = (IRUSER_Packet*)malloc(buffer_info.valid_packet_count * sizeof(IRUSER_Packet));

    int failed = 0; // number of bad packets
    // Parse the packets
    for (size_t i = 0; i < buffer_info.valid_packet_count; i++) {
        // bad packet
        if (!iruserParsePacket((i + buffer_info.start_index) % iruserRecvPacketCount, &packets[i - failed])) {
            failed++; // retry with next packet
        }
    }
    
    *n -= failed; // update the number of packets
    
    // all packets were bad
    if (*n == 0) {
        free(packets);
        return NULL;
    }
    return packets;
}

/// Circle Pad Pro specific request.
///
/// This will send a packet to the CPP requesting it to send back packets
/// with the current device input values.
Result iruserCPPRequestInputPolling(u8 period_ms) {
    u8 ir_request[3] = {
        1,
        period_ms,
        (u8)((period_ms + 2) << 2)
    };
    return IRUSER_SendIrNop(3, ir_request);
}

Handle iruserGetServHandle() {
    return iruserHandle;
}
