/**
 * @file pxidev.h
 * @brief Gamecard PXI service.
 */
#pragma once

#include <3ds/services/fs.h>

/// Card SPI wait operation type.
typedef enum {
	WAIT_NONE          = 0, ///< Do not wait.
	WAIT_SLEEP         = 1, ///< Sleep for the specified number of nanoseconds.
	WAIT_IREQ_RETURN   = 2, ///< Wait for IREQ, return if timeout.
	WAIT_IREQ_CONTINUE = 3  ///< Wait for IREQ, continue if timeout.
} PXIDEV_WaitType;

/// Card SPI register deassertion type.
typedef enum {
	DEASSERT_NONE        = 0, ///< Do not deassert.
	DEASSERT_BEFORE_WAIT = 1, ///< Deassert before waiting.
	DEASSERT_AFTER_WAIT  = 2  ///< Deassert after waiting.
} PXIDEV_DeassertType;

/// Card SPI transfer buffer.
typedef struct {
	void* ptr;         ///< Data pointer.
	u32 size;          ///< Data size.
	u8 transferOption; ///< Transfer options. See @ref pxiDevMakeTransferOption
	u64 waitOperation; ///< Wait operation. See @ref pxiDevMakeWaitOperation
} PXIDEV_SPIBuffer;

/// Initializes pxi:dev.
Result pxiDevInit(void);

/// Shuts down pxi:dev.
void pxiDevExit(void);

/**
 * @brief Creates a packed card SPI transfer option value.
 * @param baudRate Baud rate to use when transferring.
 * @param busMode Bus mode to use when transferring.
 * @return A packed card SPI transfer option value.
 */
static inline u8 pxiDevMakeTransferOption(FS_CardSpiBaudRate baudRate, FS_CardSpiBusMode busMode)
{
	return (baudRate & 0x3F) | ((busMode & 0x3) << 6);
}

/**
 * @brief Creates a packed card SPI wait operation value.
 * @param waitType Type of wait to perform.
 * @param deassertType Type of register deassertion to perform.
 * @param timeout Timeout, in nanoseconds, to wait, if applicable.
 * @return A packed card SPI wait operation value.
 */
static inline u64 pxiDevMakeWaitOperation(PXIDEV_WaitType waitType, PXIDEV_DeassertType deassertType, u64 timeout)
{
	return (waitType & 0xF) | ((deassertType & 0xF) << 4) | ((timeout & 0xFFFFFFFFFFFFFF) << 8);
}

/**
 * @brief Performs multiple card SPI writes and reads.
 * @param header Header to lead the transfers with. Must be, at most, 8 bytes in size.
 * @param writeBuffer1 Buffer to make first transfer from.
 * @param readBuffer1 Buffer to receive first response to.
 * @param writeBuffer2 Buffer to make second transfer from.
 * @param readBuffer2 Buffer to receive second response to.
 * @param footer Footer to follow the transfers with. Must be, at most, 8 bytes in size. Wait operation is unused.
 */
Result PXIDEV_SPIMultiWriteRead(PXIDEV_SPIBuffer* header, PXIDEV_SPIBuffer* writeBuffer1, PXIDEV_SPIBuffer* readBuffer1, PXIDEV_SPIBuffer* writeBuffer2, PXIDEV_SPIBuffer* readBuffer2, PXIDEV_SPIBuffer* footer);

/**
 * @brief Performs a single card SPI write and read.
 * @param bytesRead Pointer to output the number of bytes received to.
 * @param initialWaitOperation Wait operation to perform before transferring data.
 * @param writeBuffer Buffer to transfer data from.
 * @param readBuffer Buffer to receive data to.
 */
Result PXIDEV_SPIWriteRead(u32* bytesRead, u64 initialWaitOperation, PXIDEV_SPIBuffer* writeBuffer, PXIDEV_SPIBuffer* readBuffer);
