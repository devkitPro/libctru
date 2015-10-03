/*
  _irrst.c - C-stick, ZL/ZR
*/
#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/mappable.h>
#include <3ds/services/irrst.h>

// used to determine whether or not we should do IRRST_Initialize
Handle __get_handle_from_list(char* name);

Handle irrstHandle;
Handle irrstMemHandle;
Handle irrstEvent;

vu32* irrstSharedMem;

static u32 kHeld;
static circlePosition csPos;
static bool irrstUsed = false;

Result irrstInit()
{
	if(irrstUsed)return 0;

	Result ret=0;

	// Request service.
	if((ret=srvGetServiceHandle(&irrstHandle, "ir:rst")))return ret;

	// Get sharedmem handle.
	if((ret=IRRST_GetHandles(&irrstMemHandle, &irrstEvent))) goto cleanup1;

	// Initialize ir:rst
	if(__get_handle_from_list("ir:rst")==0)ret=IRRST_Initialize(10, 0);

	// Map ir:rst shared memory.
	irrstSharedMem=(vu32*)mappableAlloc(0x98);
	if(!irrstSharedMem)
	{
		ret = -1;
		goto cleanup1;
	}

	if((ret=svcMapMemoryBlock(irrstMemHandle, (u32)irrstSharedMem, MEMPERM_READ, 0x10000000)))goto cleanup2;

	// Reset internal state.
	irrstUsed = true;
	kHeld = 0;
	return 0;

cleanup2:
	svcCloseHandle(irrstMemHandle);
	if(irrstSharedMem != NULL)
	{
		mappableFree((void*) irrstSharedMem);
		irrstSharedMem = NULL;
	}
cleanup1:
	svcCloseHandle(irrstHandle);
	return ret;
}

void irrstExit()
{
	if(!irrstUsed)return;

	irrstUsed = false;
	svcCloseHandle(irrstEvent);
	// Unmap ir:rst sharedmem and close handles.
	svcUnmapMemoryBlock(irrstMemHandle, (u32)irrstSharedMem);
	if(__get_handle_from_list("ir:rst")==0) IRRST_Shutdown();
	svcCloseHandle(irrstMemHandle);
	svcCloseHandle(irrstHandle);

	if(irrstSharedMem != NULL)
	{
		mappableFree((void*) irrstSharedMem);
		irrstSharedMem = NULL;
	}
}

void irrstWaitForEvent(bool nextEvent)
{
	if(nextEvent)svcClearEvent(irrstEvent);
	svcWaitSynchronization(irrstEvent, U64_MAX);
	if(!nextEvent)svcClearEvent(irrstEvent);
}

u32 irrstCheckSectionUpdateTime(vu32 *sharedmem_section, u32 id)
{
	s64 tick0=0, tick1=0;

	if(id==0)
	{
		tick0 = *((u64*)&sharedmem_section[0]);
		tick1 = *((u64*)&sharedmem_section[2]);

		if(tick0==tick1 || tick0<0 || tick1<0)return 1;
	}

	return 0;
}

void irrstScanInput()
{
	if(!irrstUsed)return;
	
	u32 Id=0;
	kHeld = 0;
	memset(&csPos, 0, sizeof(circlePosition));

	Id = irrstSharedMem[4]; //PAD / circle-pad
	if(Id>7)Id=7;
	if(irrstCheckSectionUpdateTime(irrstSharedMem, Id)==0)
	{
		kHeld = irrstSharedMem[6 + Id*4];
		csPos = *(circlePosition*)&irrstSharedMem[6 + Id*4 + 3];
	}
}

u32 irrstKeysHeld()
{
	if(irrstUsed)return kHeld;
	return 0;
}

void irrstCstickRead(circlePosition* pos)
{
	if (pos) *pos = csPos;
}

Result IRRST_GetHandles(Handle* outMemHandle, Handle* outEventHandle)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00010000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(irrstHandle)))return ret;

	if(outMemHandle)*outMemHandle=cmdbuf[3];
	if(outEventHandle)*outEventHandle=cmdbuf[4];

	return cmdbuf[1];
}

Result IRRST_Initialize(u32 unk1, u8 unk2)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00020080; //request header code
	cmdbuf[1]=unk1;
	cmdbuf[2]=unk2;

	Result ret=0;
	if((ret=svcSendSyncRequest(irrstHandle)))return ret;

	return cmdbuf[1];
}

Result IRRST_Shutdown(void)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00030000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(irrstHandle)))return ret;

	return cmdbuf[1];
}
