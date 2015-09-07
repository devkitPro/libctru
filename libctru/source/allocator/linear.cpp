extern "C"
{
	#include <3ds/types.h>
	#include <3ds/linear.h>
	#include <3ds/util/rbtree.h>
}

#include "mem_pool.h"
#include "addrmap.h"

extern u32 __linear_heap, __linear_heap_size;

static MemPool sLinearPool;

static bool linearInit()
{
	auto blk = MemBlock::Create((u8*)__linear_heap, __linear_heap_size);
	if (blk)
	{
		sLinearPool.AddBlock(blk);
		rbtree_init(&sAddrMap, addrMapNodeComparator);
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

	// Allocate the chunk
	MemChunk chunk;
	if (!sLinearPool.Allocate(chunk, size, shift))
		return nullptr;

	auto node = newNode(chunk);
	if (!node)
	{
		sLinearPool.Deallocate(chunk);
		return nullptr;
	}
	if (rbtree_insert(&sAddrMap, &node->node));
	return chunk.addr;
}

void* linearAlloc(size_t size)
{
	return linearMemAlign(size, 0x80);
}

void* linearRealloc(void* mem, size_t size)
{
	// TODO
	return NULL;
}

void linearFree(void* mem)
{
	auto node = getNode(mem);
	if (!node) return;

	// Free the chunk
	sLinearPool.Deallocate(node->chunk);

	// Free the node
	delNode(node);
}

u32 linearSpaceFree()
{
	return sLinearPool.GetFreeSpace();
}
