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
    
	cmdbuf[0] = IPC_MakeHeader(0x6, 3, 0); // 0x000600C0
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

Result IRUSER_GetLatestReceiveErrorResult() {
    return MAKERESULT(RL_INFO, RS_NOTSUPPORTED, RM_IR, RD_NOT_IMPLEMENTED);
}

Result IRUSER_GetLatestSendErrorResult() {
    return MAKERESULT(RL_INFO, RS_NOTSUPPORTED, RM_IR, RD_NOT_IMPLEMENTED);
}

Result IRUSER_GetConnectionStatus() {
    return MAKERESULT(RL_INFO, RS_NOTSUPPORTED, RM_IR, RD_NOT_IMPLEMENTED);
}

Result IRUSER_GetTryingToConnectStatus() {
    return MAKERESULT(RL_INFO, RS_NOTSUPPORTED, RM_IR, RD_NOT_IMPLEMENTED);
}

Result IRUSER_GetReceiveSizeFreeAndUsed() {
    return MAKERESULT(RL_INFO, RS_NOTSUPPORTED, RM_IR, RD_NOT_IMPLEMENTED);
}

Result IRUSER_GetSendSizeFreeAndUsed() {
    return MAKERESULT(RL_INFO, RS_NOTSUPPORTED, RM_IR, RD_NOT_IMPLEMENTED);
}

Result IRUSER_GetConnectionRole() {
    return MAKERESULT(RL_INFO, RS_NOTSUPPORTED, RM_IR, RD_NOT_IMPLEMENTED);
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

Result iruserGetCirclePadProState(circlePadProInputResponse* response) {
    Result ret;
    IRUSER_Packet* packet = iruserGetPackets(&ret);
    if (R_FAILED(ret)) return ret;
    if (packet->payload_length != 6) return MAKERESULT(RL_TEMPORARY, RS_INVALIDSTATE, RM_IR, RD_INVALID_SIZE);
    if (packet->payload[0] != CIRCLE_PAD_PRO_INPUT_RESPONSE_PACKET_ID) return MAKERESULT(RL_TEMPORARY, RS_INVALIDSTATE, RM_IR, RD_INVALID_ENUM_VALUE);
    response->cstick.csPos.dx = (u16)(packet->payload[1] | ((packet->payload[2] & 0x0F) << 8));
    response->cstick.csPos.dy = (u16)(((packet->payload[2] & 0xF0) >> 4) | ((packet->payload[3]) << 4));
    response->status_raw = packet->payload[4];
    response->unknown_field = packet->payload[5];
    free(packet);
    return MAKERESULT(RL_SUCCESS, RS_SUCCESS, RM_COMMON, RD_SUCCESS);
}

Result iruserCirclePadProRead(circlePosition *pos) {
    Result ret;
    IRUSER_Packet* packet = iruserGetPackets(&ret);
    if (R_FAILED(ret)) return ret;
    if (packet->payload_length != 6) return RS_INVALIDSTATE;
    if (packet->payload[0] != CIRCLE_PAD_PRO_INPUT_RESPONSE_PACKET_ID) return RS_INVALIDSTATE;
    pos->dx = (s16)(packet->payload[1] | ((packet->payload[2] & 0x0F) << 8));
    pos->dy = (s16)(((packet->payload[2] & 0xF0) >> 4) | ((packet->payload[3]) << 4));
    
    free(packet);
    return RS_SUCCESS;
}

/// Read and parse the current packets received from the IR device.
IRUSER_Packet* iruserGetPackets(Result* res) {
    void* shared_mem = iruserSharedMem;

    // Find where the p1ackets are, and how many
    u32 start_index = *(u32*)((u8*)shared_mem + 0x10);
    u32 valid_packet_count = *(u32*)((u8*)shared_mem + 0x18);
    
    IRUSER_Packet* packets = (IRUSER_Packet*)malloc(valid_packet_count * sizeof(IRUSER_Packet));

    // Parse the packets
    for (size_t i = 0; i < valid_packet_count; i++) {
        u32 packet_index = (i + start_index) % iruserRecvPacketCount;
        u32 packet_info_offset = SHARED_MEM_RECV_BUFFER_OFFSET + (packet_index * PACKET_INFO_SIZE);
        u8* packet_info = (u8*)shared_mem + packet_info_offset;
        u32 offset_to_data_buffer = *(u32*)packet_info;
        u32 data_length = *(u32*)(packet_info + 4);
        u32 packet_info_section_size = iruserRecvPacketCount * PACKET_INFO_SIZE;
        u32 header_size = SHARED_MEM_RECV_BUFFER_OFFSET + packet_info_section_size;
        u32 data_buffer_size = iruserRecvBufferSize - packet_info_section_size;
        
        u8 packet_data(u32 idx) {
            u32 data_buffer_offset = offset_to_data_buffer + idx;
            return *(u8*)((u8*)shared_mem + header_size + data_buffer_offset % data_buffer_size);
        }
        
        u32 payload_length, payload_offset;
        if ((packet_data(2) & 0x40 )!= 0) {
            // Big payload
            payload_length = ((packet_data(2) & 0x3F) << 8) + packet_data(3);
            payload_offset = 4;
        } else {
            // Small payload
            payload_length = packet_data(2) & 0x3F;
            payload_offset = 3;
        }
        if (data_length != payload_offset + payload_length + 1) {
            *res = RS_INVALIDSTATE;
            return NULL;
        }
        if (packet_data(0) != 0xA5) {
            *res = RS_INVALIDSTATE;
            return NULL;
        }
        
        packets[i].magic_number = packet_data(0);
        packets[i].destination_network_id = packet_data(1);
        packets[i].payload_length = payload_length;
        packets[i].payload = (u8*)payload_offset;
        packets[i].checksum = packet_data(payload_offset + payload_length);
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
