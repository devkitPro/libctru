/**
 * @file vram.h
 * @brief VRAM allocator.
 */
#pragma once

typedef enum
{
	VRAM_A  = 0x00;
	VRAM_B  = 0x01;
	VRAM_AB = 0x02;
} VRAM_ALLOCATOR;

/**
 * @brief Allocates a 0x80-byte aligned buffer to a specific VRAM bank.
 * @param bank The bank to which the buffer will reside.
 * @param size Size of the buffer to allocate.
 * @return The allocated buffer.
 */
void* vramBankAlloc(VRAM_ALLOCATOR bank, size_t size);

/**
 * @brief Allocates a 0x80-byte aligned buffer.
 * @param size Size of the buffer to allocate.
 * @return The allocated buffer.
 */
void* vramAlloc(size_t size);

/**
 * @brief Allocates a buffer aligned to the given size in a specific bank.
 * @param bank The bank to which the buffer will reside
 * @param size Size of the buffer to allocate.
 * @param alignment Alignment to use.
 * @return The allocated buffer.
 */
void* vramBankMemAlign(VRAM_ALLOCATOR bank, size_t size, size_t alignment);

/**
 * @brief Allocates a buffer aligned to the given size.
 * @param size Size of the buffer to allocate.
 * @param alignment Alignment to use.
 * @return The allocated buffer.
 */
void* vramMemAlign(size_t size, size_t alignment);

/**
 * @brief Reallocates a buffer.
 * Note: Not implemented yet.
 * @param mem Buffer to reallocate.
 * @param size Size of the buffer to allocate.
 * @return The reallocated buffer.
 */
void* vramRealloc(void* mem, size_t size);

/**
 * @brief Frees a buffer.
 * @param mem Buffer to free.
 */
void vramFree(void* mem);

/**
 * @brief Gets the current VRAM bank free space.
 * @return The current VRAM bank free space.
 */
u32 vramBankSpaceFree(VRAM_ALLOCATOR bank);

/**
 * @brief Gets the current overall VRAM free space.
 * @return The current overall VRAM free space.
 */
u32 vramSpaceFree(void);