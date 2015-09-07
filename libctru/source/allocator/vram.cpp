extern "C"
{
	#include <3ds/types.h>
	#include <3ds/vram.h>
	#include <3ds/util/rbtree.h>
}

#include "mem_pool.h"
#include "addrmap.h"

static MemPool sVramPool;

static bool vramInit()
{
	auto blk = MemBlock::Create((u8*)0x1F000000, 0x00600000);
	if (blk)
	{
		sVramPool.AddBlock(blk);
		rbtree_init(&sAddrMap, addrMapNodeComparator);
		return true;
	}
	return false;
}

void* vramMemAlign(size_t size, size_t alignment)
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
	if (!sVramPool.Ready() && !vramInit())
		return nullptr;

	// Allocate the chunk
	MemChunk chunk;
	if (!sVramPool.Allocate(chunk, size, shift))
		return nullptr;

	auto node = newNode(chunk);
	if (!node)
	{
		sVramPool.Deallocate(chunk);
		return nullptr;
	}
	if (rbtree_insert(&sAddrMap, &node->node));
	return chunk.addr;
}

void* vramAlloc(size_t size)
{
	return vramMemAlign(size, 0x80);
}

void* vramRealloc(void* mem, size_t size)
{
	// TODO
	return NULL;
}

void vramFree(void* mem)
{
	auto node = getNode(mem);
	if (!node) return;

	// Free the chunk
	sVramPool.Deallocate(node->chunk);

	// Free the node
	delNode(node);
}

u32 vramSpaceFree()
{
	return sVramPool.GetFreeSpace();
}
