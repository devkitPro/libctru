#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/srv.h>
#include <ctr/APT.h>
#include <ctr/GSP.h>
#include <ctr/HID.h>
#include <ctr/svc.h>
#include "costable.h"

u8* gspHeap;
u32* gxCmdBuf;

u8 currentBuffer;
u8* topLeftFramebuffers[2];

void gspGpuInit()
{
	gspInit();

	GSPGPU_AcquireRight(NULL, 0x0);
	GSPGPU_SetLcdForceBlack(NULL, 0x0);

	//set subscreen to blue
	u32 regData=0x01FF0000;
	GSPGPU_WriteHWRegs(NULL, 0x202A04, (u8*)&regData, 4);

	//grab main left screen framebuffer addresses
	GSPGPU_ReadHWRegs(NULL, 0x400468, (u8*)&topLeftFramebuffers, 8);

	//convert PA to VA (assuming FB in VRAM)
	topLeftFramebuffers[0]+=0x7000000;
	topLeftFramebuffers[1]+=0x7000000;

	//setup our gsp shared mem section
	u8 threadID;
	Handle gspEvent, gspSharedMemHandle;
	svc_createEvent(&gspEvent, 0x0);
	GSPGPU_RegisterInterruptRelayQueue(NULL, gspEvent, 0x1, &gspSharedMemHandle, &threadID);
	svc_mapMemoryBlock(gspSharedMemHandle, 0x10002000, 0x3, 0x10000000);

	//map GSP heap
	svc_controlMemory((u32*)&gspHeap, 0x0, 0x0, 0x2000000, 0x10003, 0x3);

	//wait until we can write stuff to it
	svc_waitSynchronization1(gspEvent, 0x55bcb0);

	//GSP shared mem : 0x2779F000
	gxCmdBuf=(u32*)(0x10002000+0x800+threadID*0x200);

	currentBuffer=0;
}

void swapBuffers()
{
	u32 regData;
	GSPGPU_ReadHWRegs(NULL, 0x400478, (u8*)&regData, 4);
	regData^=1;
	currentBuffer=regData&1;
	GSPGPU_WriteHWRegs(NULL, 0x400478, (u8*)&regData, 4);
}

void copyBuffer()
{
	//copy topleft FB
	u8 copiedBuffer=currentBuffer^1;
	u8* bufAdr=&gspHeap[0x46500*copiedBuffer];
	GSPGPU_FlushDataCache(NULL, bufAdr, 0x46500);
	//GX RequestDma
	u32 gxCommand[0x8];
	gxCommand[0]=0x00; //CommandID
	gxCommand[1]=(u32)bufAdr; //source address
	gxCommand[2]=(u32)topLeftFramebuffers[copiedBuffer]; //destination address
	gxCommand[3]=0x46500; //size
	gxCommand[4]=gxCommand[5]=gxCommand[6]=gxCommand[7]=0x0;

	GSPGPU_submitGxCommand(gxCmdBuf, gxCommand, NULL);
}

s32 pcCos(u16 v)
{
	return costable[v&0x1FF];
}

void renderEffect()
{
	u8* bufAdr=&gspHeap[0x46500*currentBuffer];

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
	
	aptInit();

	gspGpuInit();

	Handle hidHandle;
	Handle hidMemHandle;
	srv_getServiceHandle(NULL, &hidHandle, "hid:USER");
	HIDUSER_GetInfo(hidHandle, &hidMemHandle);
	svc_mapMemoryBlock(hidMemHandle, 0x10000000, 0x1, 0x10000000);

	HIDUSER_Init(hidHandle);

	aptSetupEventHandler();

	while(!aptGetStatus())
	{
		u32 PAD=((u32*)0x10000000)[7];
		renderEffect();
		swapBuffers();
		copyBuffer();
		u32 regData=PAD|0x01000000;
		GSPGPU_WriteHWRegs(NULL, 0x202A04, (u8*)&regData, 4);
		svc_sleepThread(1000000000);
	}

	aptExit();
	svc_exitProcess();
	return 0;
}
