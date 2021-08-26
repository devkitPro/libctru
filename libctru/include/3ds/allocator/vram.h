/**
 * @file vram.h
 * @brief VRAM allocator.
 */
#pragma once

typedef enum vramAllocPos
{
	VRAM_ALLOC_A   = BIT(0),
	VRAM_ALLOC_B   = BIT(1),
	VRAM_ALLOC_ANY = VRAM_ALLOC_A | VRAM_ALLOC_B,
} vramAllocPos;

/**
 * @brief Allocates a 0x80-byte aligned buffer.
 * @param size Size of the buffer to allocate.
 * @return The allocated buffer.
 */
void* vramAlloc(size_t size);

/**
 * @brief Allocates a 0x80-byte aligned buffer in the given VRAM bank.
 * @param size Size of the buffer to allocate.
 * @param pos VRAM bank to use (see \ref vramAllocPos).
 * @return The allocated buffer.
 */
void* vramAllocAt(size_t size, vramAllocPos pos);

/**
 * @brief Allocates a buffer aligned to the given size.
 * @param size Size of the buffer to allocate.
 * @param alignment Alignment to use.
 * @return The allocated buffer.
 */
void* vramMemAlign(size_t size, size_t alignment);

/**
 * @brief Allocates a buffer aligned to the given size in the given VRAM bank.
 * @param size Size of the buffer to allocate.
 * @param alignment Alignment to use.
 * @param pos VRAM bank to use (see \ref vramAllocPos).
 * @return The allocated buffer.
 */
void* vramMemAlignAt(size_t size, size_t alignment, vramAllocPos pos);

/**
 * @brief Reallocates a buffer.
 * Note: Not implemented yet.
 * @param mem Buffer to reallocate.
 * @param size Size of the buffer to allocate.
 * @return The reallocated buffer.
 */
void* vramRealloc(void* mem, size_t size);

/**
 * @brief Retrieves the allocated size of a buffer.
 * @return The size of the buffer.
 */
size_t vramGetSize(void* mem);

/**
 * @brief Frees a buffer.
 * @param mem Buffer to free.
 */
void vramFree(void* mem);

/**
 * @brief Gets the current VRAM free space.
 * @return The current VRAM free space.
 */
u32 vramSpaceFree(void);
