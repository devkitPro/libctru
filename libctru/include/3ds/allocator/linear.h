/**
 * @file linear.h
 * @brief Linear memory allocator.
 */
#pragma once

/**
 * @brief Allocates a 0x80-byte aligned buffer.
 * @param size Size of the buffer to allocate.
 * @return The allocated buffer.
 */
void* linearAlloc(size_t size);

/**
 * @brief Allocates a buffer aligned to the given size.
 * @param size Size of the buffer to allocate.
 * @param alignment Alignment to use.
 * @return The allocated buffer.
 */
void* linearMemAlign(size_t size, size_t alignment);

/**
 * @brief Reallocates a buffer.
 * Note: Not implemented yet.
 * @param mem Buffer to reallocate.
 * @param size Size of the buffer to allocate.
 * @return The reallocated buffer.
 */
void* linearRealloc(void* mem, size_t size);

/**
 * @brief Frees a buffer.
 * @param mem Buffer to free.
 */
void linearFree(void* mem);

/**
 * @brief Gets the current linear free space.
 * @return The current linear free space.
 */
u32 linearSpaceFree(void);
