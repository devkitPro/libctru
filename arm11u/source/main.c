#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/srv.h>
#include <ctr/APT.h>
#include <ctr/GSP.h>
#include <ctr/svc.h>

int main()
{
	Handle srvHandle, aptuHandle;
	
	getSrvHandle(&srvHandle);
	
	//initialize APT stuff, escape load screen
	srv_getServiceHandle(srvHandle, &aptuHandle, "APT:U");
	APT_GetLockHandle(aptuHandle, 0x0, NULL);
	svc_closeHandle(aptuHandle);
	svc_sleepThread(0x50000);
	
	Handle hmEvent;
	srv_getServiceHandle(srvHandle, &aptuHandle, "APT:U");
	APT_Initialize(aptuHandle, 0x300, &hmEvent, NULL);
	svc_closeHandle(aptuHandle);
	svc_sleepThread(0x50000);
	
	srv_getServiceHandle(srvHandle, &aptuHandle, "APT:U");
	APT_Enable(aptuHandle, 0x0);
	svc_closeHandle(aptuHandle);
	svc_sleepThread(0x50000);

	//do stuff with GPU...
	Handle gspGpuHandle;
	srv_getServiceHandle(srvHandle, &gspGpuHandle, "gsp::Gpu");

	GSPGPU_AcquireRight(gspGpuHandle, 0x0);
	GSPGPU_SetLcdForceBlack(gspGpuHandle, 0x0);

	//set subscreen to blue
	u32 regData=0x01FF0000;
	GSPGPU_WriteHWRegs(gspGpuHandle, 0x202A04, (u8*)&regData, 4);

	//grab main left screen framebuffer addresses
	u8* topLeftFramebuffers[2];
	GSPGPU_ReadHWRegs(gspGpuHandle, 0x400468, (u8*)&topLeftFramebuffers, 8);

	//convert PA to VA (assuming FB in VRAM)
	topLeftFramebuffers[0]+=0x7000000;
	topLeftFramebuffers[1]+=0x7000000;

	//setup our gsp shared mem section
	u8 threadID;
	Handle gspEvent, gspSharedMemHandle;
	svc_createEvent(&gspEvent, 0x0);
	GSPGPU_RegisterInterruptRelayQueue(gspGpuHandle, gspEvent, 0x1, &gspSharedMemHandle, &threadID);
	svc_mapMemoryBlock(gspSharedMemHandle, 0x10002000, 0x3, 0x10000000);

	//map GSP heap
	u8* gspHeap;
	svc_controlMemory((u32*)&gspHeap, 0x0, 0x0, 0x2000000, 0x10003, 0x3);

	int i;
	for(i=1;i<0x600000;i++)
	{
		gspHeap[i]=0xFF^gspHeap[i-1];
	}

	//wait until we can write stuff to it
	svc_waitSynchronization1(gspEvent, 0x55bcb0);

	// //GSP shared mem : 0x2779F000
	// //write GX command ! (to GSP shared mem, at 0x10002000)
	u32* gxCmdBuf=(u32*)(0x10002000+0x800+threadID*0x200);
	u32 gxCommand[0x8];

	//GX RequestDma
	gxCommand[0]=0x00; //CommandID
	gxCommand[1]=(u32)gspHeap; //source address
	// gxCommand[2]=(u32)topLeftFramebuffers[0]; //destination address
	gxCommand[2]=0x1F000000; //destination address
	// gxCommand[3]=0x5DC00*2; //size
	gxCommand[3]=0x600000; //size
	gxCommand[4]=gxCommand[5]=gxCommand[6]=gxCommand[7]=0x0;

	GSPGPU_submitGxCommand(gxCmdBuf, gxCommand, gspGpuHandle);

	// debug
	regData=0x010000FF;
	GSPGPU_WriteHWRegs(gspGpuHandle, 0x202A04, (u8*)&regData, 4);

	svc_waitSynchronization1(hmEvent, 0xffffffffffffffff);

	// debug
	regData=0x0100FFFF;
	GSPGPU_WriteHWRegs(gspGpuHandle, 0x202A04, (u8*)&regData, 4);

	while(1);

	return 0;
}
