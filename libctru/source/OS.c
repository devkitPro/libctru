#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/svc.h>
#include <ctr/OS.h>

u32 OS_ConvertVaddr2Physaddr(u32 vaddr)
{
	if(vaddr >= 0x14000000 && vaddr<0x1c000000)return vaddr + 0x0c000000;//LINEAR memory
	if(vaddr >= 0x30000000 && vaddr<0x40000000)return vaddr - 0x10000000;//Only available under system-version v8.0 for certain processes, see here: http://3dbrew.org/wiki/SVC#enum_MemoryOperation
	if(vaddr >= 0x1F000000 && vaddr<0x1F600000)return vaddr - 0x07000000;//VRAM

	return 0;
}

