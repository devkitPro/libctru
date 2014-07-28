#include <3ds/types.h>
#include <3ds/srv.h>
#include <3ds/APT.h>
#include <3ds/GSP.h>
#include <3ds/GX.h>
#include <3ds/HID.h>
#include <3ds/FS.h>
#include <3ds/svc.h>
#include "costable.h"

u8* gspHeap;
u32* gxCmdBuf;

u8 currentBuffer;
u8* topLeftFramebuffers[2];

Handle gspEvent, gspSharedMemHandle;

void gspGpuInit()
{
	gspInit();

	GSPGPU_AcquireRight(NULL, 0x0);
	GSPGPU_SetLcdForceBlack(NULL, 0x0);

	//set subscreen to blue
	u32 regData=0x01FF0000;
	GSPGPU_WriteHWRegs(NULL, 0x202A04, &regData, 4);

	//grab main left screen framebuffer addresses
	GSPGPU_ReadHWRegs(NULL, 0x400468, (u32*)&topLeftFramebuffers, 8);

	//convert PA to VA (assuming FB in VRAM)
	topLeftFramebuffers[0]+=0x7000000;
	topLeftFramebuffers[1]+=0x7000000;

	//setup our gsp shared mem section
	u8 threadID;
	svcCreateEvent(&gspEvent, 0x0);
	GSPGPU_RegisterInterruptRelayQueue(NULL, gspEvent, 0x1, &gspSharedMemHandle, &threadID);
	svcMapMemoryBlock(gspSharedMemHandle, 0x10002000, 0x3, 0x10000000);

	//map GSP heap
	svcControlMemory((u32*)&gspHeap, 0x0, 0x0, 0x2000000, 0x10003, 0x3);

	//wait until we can write stuff to it
	svcWaitSynchronization1(gspEvent, 0x55bcb0);

	//GSP shared mem : 0x2779F000
	gxCmdBuf=(u32*)(0x10002000+0x800+threadID*0x200);

	currentBuffer=0;
}

void gspGpuExit()
{
	GSPGPU_UnregisterInterruptRelayQueue(NULL);

	//unmap GSP shared mem
	svcUnmapMemoryBlock(gspSharedMemHandle, 0x10002000);
	svcCloseHandle(gspSharedMemHandle);
	svcCloseHandle(gspEvent);
	
	gspExit();

	//free GSP heap
	svcControlMemory((u32*)&gspHeap, (u32)gspHeap, 0x0, 0x2000000, MEMOP_FREE, 0x0);
}


void swapBuffers()
{
	u32 regData;
	GSPGPU_ReadHWRegs(NULL, 0x400478, &regData, 4);
	regData^=1;
	currentBuffer=regData&1;
	GSPGPU_WriteHWRegs(NULL, 0x400478, &regData, 4);
}

void copyBuffer()
{
	//copy topleft FB
	u8 copiedBuffer=currentBuffer^1;
	u8* bufAdr=&gspHeap[0x46500*copiedBuffer];
	GSPGPU_FlushDataCache(NULL, bufAdr, 0x46500);

	GX_RequestDma(gxCmdBuf, (u32*)bufAdr, (u32*)topLeftFramebuffers[copiedBuffer], 0x46500);
}

s32 pcCos(u16 v)
{
	return costable[v&0x1FF];
}

void renderEffect()
{
	u8* bufAdr=&gspHeap[0x46500*currentBuffer];

	if(!currentBuffer)return;
	int i, j;
	for(i=1;i<400;i++)
	{
		for(j=1;j<240;j++)
		{
			u32 v=(j+i*240)*3;
			bufAdr[v]=(pcCos(i)+4096)/32;
			bufAdr[v+1]=0x00;
			bufAdr[v+2]=0xFF*currentBuffer;
		}
	}
}

int main()
{
	initSrv();
		
	aptInit(APPID_APPLICATION);

	gspGpuInit();

	hidInit(NULL);

	Handle fsuHandle;
	srv_getServiceHandle(NULL, &fsuHandle, "fs:USER");
	FSUSER_Initialize(fsuHandle);

	Handle fileHandle;
	u32 bytesRead;
	FS_archive sdmcArchive=(FS_archive){0x9, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	FS_path filePath=(FS_path){PATH_CHAR, 10, (u8*)"/test.bin"};
	FSUSER_OpenFileDirectly(fsuHandle, &fileHandle, sdmcArchive, filePath, FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	FSFILE_Read(fileHandle, &bytesRead, 0x0, (u32*)gspHeap, 0x46500);
	FSFILE_Close(fileHandle);

	aptSetupEventHandler();
	
	APP_STATUS status;
	while((status=aptGetStatus())!=APP_EXITING)
	{
		if(status==APP_RUNNING)
		{
			u32 PAD=hidSharedMem[7];
			renderEffect();
			swapBuffers();
			copyBuffer();
			u32 regData=PAD|0x01000000;
			GSPGPU_WriteHWRegs(NULL, 0x202A04, &regData, 4);
			svcSleepThread(1000000000);
		}
		else if(status == APP_SUSPENDING)
		{
			aptReturnToMenu();
		}
		else if(status == APP_SLEEPMODE)
		{
			aptWaitStatusEvent();
		}
	}

	svcCloseHandle(fsuHandle);
	hidExit();
	gspGpuExit();
	aptExit();
	svcExitProcess();
	return 0;
}
