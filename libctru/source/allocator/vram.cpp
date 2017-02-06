extern "C"
{
	#include <3ds/types.h>
	#include <3ds/allocator/vram.h>
	#include <3ds/util/rbtree.h>
}

#include "mem_pool.h"
#include "addrmap.h"

static MemPool sVramPoolA;
static MemPool sVramPoolB;

static bool vramInit()
{
	auto blkA = MemBlock::Create((u8*)0x1F000000, 0x00300000);
	if (blkA)
	{
		auto blkB = MemBlock::Create((u8*)0x1F300000, 0x00300000);
		if(blkB)
		{
			sVramPoolA.AddBlock(blkA);
			sVramPoolB.Addblock(blkB);
			rbtree_init(&sAddrMap, addrMapNodeComparator);
			return true;
		}
		free(blkA);
	}
	return false;
}

void* vramBankMemAlign(VRAM_ALLOCATOR bank, size_t size, size_t alignment)
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
	if (!sVramPoolA.Ready() && !sVramPoolB.Ready() && !vramInit())
		return nullptr;

	// Allocate the chunk
	MemChunk chunk;
	int bankSet;
	if(bank != VRAM_B)
	{
		// Attempt Bank A (for both cases)
		if(!sVramPoolA.Allocate(chunk, size, shift))
		{
			if(bank == VRAM_A)
				return nullptr;
			else
			{
				// Attempt Bank B
				if(!sVramPoolB.Allocate(chunk, size, shift))
					return nullptr;
				bankSet = 1;
			}
		}
		else
			bankset = 0;
	}
	else
	{
		// Attempt Bank B only
		if(!sVramPoolB.Allocate(chunk, size, shift))
			return nullptr;
		bankSet = 1;
	}

	auto node = newNode(chunk);
	if (!node)
	{
		if(bankSet)
			sVramPoolB.Deallocate(chunk);
		else
			sVramPoolA.Deallocate(chunk);
		return nullptr;
	}

	if (rbtree_insert(&sAddrMap, &node->node));
	return chunk.addr;
}

void* vramMemAlign(size_t size, size_t alignment)
{
	return vramBankMemAlign(VRMA_AB, size, alignment);
}

void* vramBankAlloc(VRAM_ALLOCATOR bank, size_t size)
{
	return vramBankMemAlign(bank, size, 0x80);
}

void* vramAlloc(size_t size)
{
	return vramBankMemAlign(VRAM_AB, size, 0x80);
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
	if((u32)mem < 0x1F300000)
		sVramPoolA.Deallocate(node->chunk);
	else
		sVramPoolB.Deallocate(node->chunk);

	// Free the node
	delNode(node);
}

u32 vramBankSpaceFree(VRAM_ALLOCATOR bank);
{
	if(bank != VRAM_B)
	{
		u32 space = sVramPoolA.GetFreeSpace();
		if(bank == VRAM_A)
			return space;
		else
			return space + sVramPoolB.GetFreeSpace();
	}
	else
		return sVramPoolB.GetFreeSpace();
}

u32 vramSpaceFree()
{
	return sVramPoolA.GetFreeSpace() + sVramPoolB.GetFreeSpace();
}
