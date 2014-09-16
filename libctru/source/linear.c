#include <3ds.h>

extern u32 __linear_heap, __linear_heap_size;

// TODO: this allocator sucks! It is not thread-safe and you cannot 'free' this memory.
void* linearAlloc(size_t size)
{
	static size_t currentOffset = 0;
	size_t free = __linear_heap_size - currentOffset;

	// Enforce 16-byte alignment
	size = (size + 15) &~ 15;

	void* mem = NULL;
	if (free >= size)
	{
		mem = (void*)(__linear_heap + currentOffset);
		currentOffset += size;
	}

	return mem;
}

void* linearRealloc(void* mem, size_t size)
{
	return NULL; // TODO
}

void linearFree(void* mem)
{
	// TODO
}
