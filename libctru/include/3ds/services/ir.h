/**
 * @file ir.h
 * @brief IR service.
 */
#pragma once

/**
 * @brief Initializes IRU.
 * The permissions for the specified memory is set to RO. This memory must be already mapped.
 * @param sharedmem_addr Address of the shared memory block to use.
 * @param sharedmem_size Size of the shared memory block.
 */
Result iruInit(u32 *sharedmem_addr, u32 sharedmem_size);

/// Shuts down IRU.
void iruExit(void);

/**
 * @brief Gets the IRU service handle.
 * @return The IRU service handle.
 */
Handle iruGetServHandle(void);

/**
 * @brief Sends IR data.
 * @param buf Buffer to send data from.
 * @param size Size of the buffer.
 * @param wait Whether to wait for the data to be sent.
 */
Result iruSendData(u8 *buf, u32 size, bool wait);

/**
 * @brief Receives IR data.
 * @param buf Buffer to receive data to.
 * @param size Size of the buffer.
 * @param flag Flags to receive data with.
 * @param transfercount Pointer to output the number of bytes read to.
 * @param wait Whether to wait for the data to be received.
 */
Result iruRecvData(u8 *buf, u32 size, u8 flag, u32 *transfercount, bool wait);

/// Initializes the IR session.
Result IRU_Initialize(void);

/// Shuts down the IR session.
Result IRU_Shutdown(void);

/**
 * @brief Begins sending data.
 * @param buf Buffer to send.
 * @param size Size of the buffer.
 */
Result IRU_StartSendTransfer(u8 *buf, u32 size);

/// Waits for a send operation to complete.
Result IRU_WaitSendTransfer(void);

/**
 * @brief Begins receiving data.
 * @param size Size of the data to receive.
 * @param flag Flags to use when receiving.
 */
Result IRU_StartRecvTransfer(u32 size, u8 flag);

/**
 * @brief Waits for a receive operation to complete.
 * @param transfercount Pointer to output the number of bytes read to.
 */
Result IRU_WaitRecvTransfer(u32 *transfercount);

/**
 * @brief Sets the IR bit rate.
 * @param value Bit rate to set.
 */
Result IRU_SetBitRate(u8 value);

/**
 * @brief Gets the IR bit rate.
 * @param out Pointer to write the bit rate to.
 */
Result IRU_GetBitRate(u8 *out);

/**
 * @brief Sets the IR LED state.
 * @param value IR LED state to set.
 */
Result IRU_SetIRLEDState(u32 value);

/**
 * @brief Gets the IR LED state.
 * @param out Pointer to write the IR LED state to.
 */
Result IRU_GetIRLEDRecvState(u32 *out);
