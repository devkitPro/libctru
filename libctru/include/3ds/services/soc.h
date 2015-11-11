/**
 * @file soc.h
 * @brief SOC service for sockets communications
 *
 * After initializing this service you will be able to use system calls from netdb.h, sys/socket.h etc.
 */
#pragma once

/**
 * @brief Initializes the SOC service.
 * @param context_addr Address of a page-aligned (0x1000) buffer to be used.
 * @param context_size Size of the buffer, a multiple of 0x1000.
 * @note The specified context buffer can no longer be accessed by the process which called this function, since the userland permissions for this block are set to no-access.
 */
Result socInit(u32 *context_addr, u32 context_size);

/**
 * @brief Closes the soc service.
 * @note You need to call this in order to be able to use the buffer again.
 */
Result socExit(void);

// this is supposed to be in unistd.h but newlib only puts it for cygwin
/**
 * @brief Gets the system's host ID.
 * @return The system's host ID.
 */
long gethostid(void);

