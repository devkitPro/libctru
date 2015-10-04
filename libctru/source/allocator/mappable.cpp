extern "C"
{
	#include <3ds/types.h>
	#include <3ds/mappable.h>
	#include <3ds/util/rbtree.h>
}

#include "mem_pool.h"
#include "addrmap.h"

static MemPool sMappablePool;

static bool mappableInit()
{
	auto blk = MemBlock::Create((u8*)0x10000000, 0x04000000);
	if (blk)
	{
		sMappablePool.AddBlock(blk);
		rbtree_init(&sAddrMap, addrMapNodeComparator);
		return true;
	}
	return false;
}

void* mappableAlloc(size_t size)
{
	// Initialize the pool if it is not ready
	if (!sMappablePool.Ready() && !mappableInit())
		return nullptr;

	// Allocate the chunk
	MemChunk chunk;
	if (!sMappablePool.Allocate(chunk, size, 12))
		return nullptr;

	auto node = newNode(chunk);
	if (!node)
	{
		sMappablePool.Deallocate(chunk);
		return nullptr;
	}
	if (rbtree_insert(&sAddrMap, &node->node));
	return chunk.addr;
}

void mappableFree(void* mem)
{
	auto node = getNode(mem);
	if (!node) return;

	// Free the chunk
	sMappablePool.Deallocate(node->chunk);

	// Free the node
	delNode(node);
}

u32 mappableSpaceFree()
{
	return sMappablePool.GetFreeSpace();
}
