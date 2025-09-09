extern "C"
{
	#include <3ds/types.h>
	#include <3ds/allocator/linear.h>
	#include <3ds/util/rbtree.h>
	#include <string.h> // for memcpy
}

#include "mem_pool.h"
#include "addrmap.h"

extern u32 __ctru_linear_heap;
extern u32 __ctru_linear_heap_size;

static MemPool sLinearPool;

static bool linearInit()
{
	auto blk = MemBlock::Create((u8*)__ctru_linear_heap, __ctru_linear_heap_size);
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
	// Convert alignment to shift
	int shift = alignmentToShift(alignment);
	if (shift < 0)
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
	auto node = getNode(mem);
	if (!node) return nullptr;

	if (!sLinearPool.Reallocate(node->chunk, size))
	{
		size_t minSize = (size < node->chunk.size) ? size : node->chunk.size;
		void* ret = linearMemAlign(size, (node->chunk.alignMask + 1));
		if (ret)
		{
			memcpy(ret, mem, minSize);
			linearFree(mem);
			return ret;
		}
		return nullptr;
	}
	return mem;
}

size_t linearGetSize(void* mem)
{
	auto node = getNode(mem);
	return node ? node->chunk.size : 0;
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
