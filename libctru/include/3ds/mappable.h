/**
 * @file mappable.h
 * @brief Mappable memory allocator.
 */
#pragma once

/**
 * @brief Allocates a page-aligned buffer.
 * @param size Size of the buffer to allocate.
 * @return The allocated buffer.
 */
void* mappableAlloc(size_t size);

/**
 * @brief Frees a buffer.
 * @param mem Buffer to free.
 */
void mappableFree(void* mem);

/**
 * @brief Gets the current mappable free space.
 * @return The current mappable free space.
 */
u32 mappableSpaceFree(void);
