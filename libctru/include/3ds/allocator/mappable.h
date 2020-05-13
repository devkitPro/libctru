/**
 * @file mappable.h
 * @brief Mappable memory allocator.
 */
#pragma once

#include <3ds/types.h>

/**
 * @brief Initializes the mappable allocator.
 * @param addrMin Minimum address.
 * @param addrMax Maxium address.
 */
void mappableInit(u32 addrMin, u32 addrMax);

/**
 * @brief Finds a mappable memory area.
 * @param size Size of the area to find.
 * @return The mappable area.
 */
void* mappableAlloc(size_t size);

/**
 * @brief Frees a mappable area (stubbed).
 * @param mem Mappable area to free.
 */
void mappableFree(void* mem);
