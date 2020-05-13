#include <3ds/allocator/mappable.h>
#include <3ds/svc.h>
#include <3ds/result.h>

static u32 minAddr, maxAddr, currentAddr;

void mappableInit(u32 addrMin, u32 addrMax)
{
	minAddr = addrMin;
	maxAddr = addrMax;
	currentAddr = minAddr;
}

static u32 mappableFindAddressWithin(u32 start, u32 end, u32 size)
{
	MemInfo info;
	PageInfo pgInfo;

	u32 addr = start;
	while (addr >= start && (addr + size) < end && (addr + size) >= addr)
	{
		if (R_FAILED(svcQueryMemory(&info, &pgInfo, addr)))
			return 0;

		if (info.state == MEMSTATE_FREE)
		{
			u32 sz = info.size - (addr - info.base_addr); // a free block might cover all the memory, etc.
			if (sz >= size)
				return addr;
		}

		addr = info.base_addr + info.size;
	}

	return 0;
}

void* mappableAlloc(size_t size)
{
	// Round up, can only allocate in page units
	size = (size + 0xFFF) & ~0xFFF;

	u32 addr = mappableFindAddressWithin(currentAddr, maxAddr, size);
	if (addr == 0)
	{
		// Need to rollover (maybe)
		addr = mappableFindAddressWithin(minAddr, currentAddr, size);
		if (addr == 0)
			return NULL;
	}

	currentAddr = addr + size >= maxAddr ? minAddr : addr + size;
	return (void *)addr;
}

void mappableFree(void* mem)
{
	(void)mem;
}
