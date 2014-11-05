#include <3ds.h>
#include "mem_pool.h"

extern u32 __linear_heap, __linear_heap_size;

static MemPool sLinearPool;

static bool linearInit()
{
	auto blk = MemBlock::Create((u8*)__linear_heap, __linear_heap_size);
	if (blk)
	{
		sLinearPool.AddBlock(blk);
		return true;
	}
	return false;
}

void* linearMemAlign(size_t size, size_t alignment)
{
	// Enforce minimum alignment
	if (alignment < 16)
		alignment = 16;

	// Convert alignment to shift amount
	int shift;
	for (shift = 4; shift < 32; shift ++)
	{
		if ((1U<<shift) == alignment)
			break;
	}
	if (shift == 32) // Invalid alignment
		return nullptr;

	// Initialize the pool if it is not ready
	if (!sLinearPool.Ready() && !linearInit())
		return nullptr;

	// Reserve memory for MemChunk structure
	size += alignment;

	// Allocate the chunk
	MemChunk chunk;
	if (!sLinearPool.Allocate(chunk, size, shift))
		return nullptr;

	// Copy the MemChunk structure and return memory
	auto addr = chunk.addr;
	*(MemChunk*)addr = chunk;
	*(u32*)(addr + alignment - sizeof(u32)) = alignment;
	return addr + alignment;
}

void* linearAlloc(size_t size)
{
	return linearMemAlign(size, 16);
}

void* linearRealloc(void* mem, size_t size)
{
	// TODO
	return NULL;
}

void linearFree(void* mem)
{
	// Find MemChunk structure and free the chunk
	u32 alignment = *((u32*)mem - 1);
	auto pChunk = (MemChunk*)((u8*)mem - alignment);
	sLinearPool.Deallocate(*pChunk);
}

u32 linearSpaceFree()
{
	return sLinearPool.GetFreeSpace();
}
