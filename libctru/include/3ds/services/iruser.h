/**
 * @file iruser.h
 * @brief IRUSER service.
 */
#pragma once

#include <3ds/types.h>
#include <3ds/services/hid.h>

/// Connection status values for [`IRUSER_StatusInfo`].
typedef enum {
    CNSTATUS_Disconnected = 0, /// Device is not connected
    CNSTATUS_Connecting = 1, /// Waiting for device to connect
    CNSTATUS_Connected = 2, /// Device is connected
    CNSTATUS_Disconnecting = 3, /// Device is disconnecting
    CNSTATUS_Unknown = 4 // /// Unknown status value
} IRUSER_ConnectionStatus;

/// Connection status values for [`IRUSER_StatusInfo`].
typedef enum {
    Unk_1 = 1,
    Unk_2 = 2,
    Unk_3 = 3,
    Unk_4 = 4 
} IRUSER_TryingToConnectStatus;

typedef enum {
    CNRole_1 = 1,
    CNRole_2 = 2,
} IRUSER_ConnectionRole;

/// This struct holds a parsed copy of the ir:USER service status (from shared memory).
typedef struct {
    /// The result of the last receive operation.
    Result recv_err_result;
    /// The result of the last send operation.
    Result send_err_result;
    /// The current connection status.
    IRUSER_ConnectionStatus connection_status;
    /// The status of the connection attempt.
    u8 trying_to_connect_status;
    /// The role of the device in the connection (value meaning is unknown).
    u8 connection_role;
    /// The machine ID of the device.
    u8 machine_id;
    /// The machine ID of the target device.
    u8 target_machine_id;
    /// The network ID of the connection.
    u8 network_id;
    /// Unknown field.
    u8 unknown_field_2;
    /// Unknown field.
    u8 unknown_field_3;
} IRUSER_StatusInfo;

typedef struct {
    u32 start_index;
    u32 end_index;
    u32 valid_packet_count;
    u32 unknown_field;
} IRUSER_BufferInfo;

typedef struct {
    u32 offset;
    u32 length;
} IRUSER_PacketInfo;

/// A packet of data sent/received to/from the IR device.
typedef struct {
    /// The magic number of the packet. Should always be 0xA5.
    u8 magic_number;
    /// The destination network ID.
    u8 destination_network_id;
    /// The length of the payload.
    size_t payload_length;
    /// The payload data.
    u8* payload;
    /// The checksum of the packet.
    u8 checksum;
} IRUSER_Packet;

/// Circle Pad Pro response packet holding the current device input signals and status.
typedef struct {
    union {
        struct {
            /// The X value of the C-stick.
            u16 c_stick_x;
            /// The Y value of the C-stick.
            u16 c_stick_y;
        };
        circlePosition csPos;
    } cstick;
    union {
        u8 status_raw;
        struct {
            /// The battery level of the Circle Pad Pro.
            u8 battery_level : 5;
            /// Whether the ZL button is pressed.
            bool zl_pressed : 1;
            /// Whether the ZR button is pressed.
            bool zr_pressed : 1;
            /// Whether the R button is pressed.
            bool r_pressed : 1;
        } status;
    };
    /// Unknown field.
    u8 unknown_field;
} circlePadProInputResponse;

/**
 * @brief Initializes IRUSER.
 * Allocates memory to use as shared_memory based on buffer_size and sets it as read only.
 * 
 */
Result iruserInit(u32 *sharedmem_addr, u32 sharedmem_size, size_t buffer_size, size_t packet_count);

/// Shuts down IRUSER. Frees shared memory.
void iruserExit();

/// Gets IRUSER service handle
Handle iruserGetServHandle();


IRUSER_StatusInfo iruserGetStatusInfo();

/** 
 * @brief Circle Pad Pro specific request.
 * This will send a packet to the CPP requesting it to send back packets with the current device input values.
 * @param period_ms Period at which to send another packet.
*/ 
Result iruserCPPRequestInputPolling(u8 period_ms);

/// This will let you directly read the ir:USER shared memory via a callback.
void iruserProcessSharedMemory(void(*process_fn)(u8*));

/**
 * @brief Gets circle pad pro inputs state
 * @param response Pointer to write data to
 */
Result iruserGetCirclePadProState(IRUSER_Packet* packet, circlePadProInputResponse* response);

/**
 * @brief Reads c-stick position from circle pad pro
 * @param pos Pointer to write data to
 */
Result iruserCirclePadProCStickRead(circlePosition* pos);

/**
 * @brief Returns the packets received from the IR device.
 * @param [out] numpackets Number of valid packets that were received
 * @return Pointer to the packets received. The caller is responsible for freeing the memory (both the packet and its payload data).
 */
IRUSER_Packet* iruserGetPackets(u32* numpackets);

/**
 * @brief Initializes the IR session
 * @brief IR uses shared memory in non-shared mode  (puts less information in shared memory)
 * @param recv_buffer_size Size of receiving buffer
 * @param recv_packet_count
 * @param send_buffer_size
 * @param send_packet_count
 * @param bitrate
 */
Result IRUSER_InitializeIrNop(size_t recv_buffer_size, size_t recv_packet_count, size_t send_buffer_size, size_t send_packet_count, u32 bitrate);

/// Shuts down the IR session
Result IRUSER_FinalizeIrNop();

/// Clears receive buffer
Result IRUSER_ClearReceiveBuffer();

/// Clears send buffer
Result IRUSER_ClearSendBuffer();

/**
 * @brief Attempts to connect to device with given id and timeout.
 * @param device_id ID of the device to connect to.
 * @param timeout Timeout in milliseconds.
 */
Result IRUSER_WaitConnection(u8 target_id, u64 timeout);

/**
 * @brief Attempts to connect to device with given id.
 * @param device_id ID of the device to connect to.
 */
Result IRUSER_RequireConnection(u8 device_id);

/// Unknown.
Result IRUSER_AutoConnection();

/// Connects to any device
Result IRUSER_AnyConnection();

/// Closes the current IR connection.
Result IRUSER_Disconnect();

/**
 * @brief Gets an event handle that activates when a packet is received.
 * @param eventhandle Pointer to write the event handle to.
 */
Result IRUSER_GetReceiveEvent(Handle* eventhandle);

/**
 * @brief Gets an event handle that activates when a packet is sent.
 * @param eventhandle Pointer to write the event handle to.
 */
Result IRUSER_GetSendEvent(Handle* eventhandle);

/**
 * @brief Gets an event handle that activates on connection status change.
 * @param eventhandle Pointer to write the event handle to.
 */
Result IRUSER_GetConnectionStatusEvent(Handle* eventhandle);

/**
 * @brief Sends data to connected device
 * @brief Should be used when `size <= 0xFC`
 * @param size Size of the buffer
 * @param inbufptr Buffer to send
 */
Result IRUSER_SendIrNop(u32 size, u8* inbufptr);

/**
 * @brief Sends data to connected device
 * @brief Should be used when `size > 0xFC`
 * @param size Size of the buffer
 * @param inbufptr Buffer to send
 */
Result IRUSER_SendIrNopLarge(u32 size, u8* inbufptr);


Result IRUSER_ReceiveIrNop(); 
Result IRUSER_ReceiveIrNopLarge();


Result IRUSER_GetLatestReceiveErrorResult();
Result IRUSER_GetLatestSendErrorResult();
Result IRUSER_GetConnectionStatus(IRUSER_ConnectionStatus* status);
Result IRUSER_GetTryingToConnectStatus(IRUSER_TryingToConnectStatus* status);
Result IRUSER_GetReceiveSizeFreeAndUsed();
Result IRUSER_GetSendSizeFreeAndUsed();
Result IRUSER_GetConnectionRole(IRUSER_ConnectionRole* role);

/**
 * @brief Initializes the IR session
 * @brief IR uses shared memory in shared mode (puts more information in shared memory)
 * @param recv_buffer_size Size of receiving buffer
 * @param recv_packet_count
 * @param send_buffer_size
 * @param send_packet_count
 * @param bitrate
 */
Result IRUSER_InitializeIrNopShared(size_t recv_buffer_size, size_t recv_packet_count, size_t send_buffer_size, size_t send_packet_count, u32 bitrate);

/**
 * @brief Mark the last `packet_count` packets as processed, so their memory in the receive buffer can be reused.
 * @param packet_count Number of packets to mark as processed
 */
Result IRUSER_ReleaseReceivedData(u32 packet_count);

/**
 * @brief Sets own ID to `id`.
 * @param id ID to set
 */
Result IRUSER_SetOwnMachineId(u8 id);
