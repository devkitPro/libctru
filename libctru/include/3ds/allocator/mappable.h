/**
 * @file mappable.h
 * @brief Mappable memory allocator.
 */
#pragma once

/**
 * @brief Reserves a mappable memory area.
 * @param size Size of the area to reserve.
 * @return The mappable area.
 */
void* mappableAlloc(size_t size);

/**
 * @brief Retrieves the allocated size of a mappable area.
 * @return The size of the mappable area.
 */
size_t mappableGetSize(void* mem);

/**
 * @brief Frees a mappable area.
 * @param mem Mappable area to free.
 */
void mappableFree(void* mem);

/**
 * @brief Gets the current mappable free space.
 * @return The current mappable free space.
 */
u32 mappableSpaceFree(void);
