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

void* linearAlloc(size_t size)
{
	// Initialize the pool if it is not ready
	if (!sLinearPool.Ready() && !linearInit())
		return nullptr;

	// Reserve memory for MemChunk structure
	size += 16;

	// Allocate the chunk
	MemChunk chunk;
	if (!sLinearPool.Allocate(chunk, size, 4)) // 16-byte alignment
		return nullptr;

	// Copy the MemChunk structure and return memory
	auto addr = chunk.addr;
	*(MemChunk*)addr = chunk;
	return addr + 16;
}

void* linearRealloc(void* mem, size_t size)
{
	// TODO
	return NULL;
}

void linearFree(void* mem)
{
	// Find MemChunk structure and free the chunk
	auto pChunk = (MemChunk*)((u8*)mem - 16);
	sLinearPool.Deallocate(*pChunk);
}
