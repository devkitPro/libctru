#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/GSP.h>
#include <ctr/svc.h>


void GSPGPU_AcquireRight(Handle handle, u8 flags)
{
	u32* svcData=svc_getData();
	svcData[0]=0x160042; //request header code
	svcData[1]=flags;
	svcData[2]=0x0;
	svcData[3]=0xffff8001;
	svc_sendSyncRequest(handle); //check return value...
}

void GSPGPU_SetLcdForceBlack(Handle handle, u8 flags)
{
	u32* svcData=svc_getData();
	svcData[0]=0xB0040; //request header code
	svcData[1]=flags;
	svc_sendSyncRequest(handle); //check return value...
}

void GSPGPU_WriteHWRegs(Handle handle, u32 regAddr, u8* data, u8 size)
{
	if(size>0x80 || !data)return;

	u32* svcData=svc_getData();
	svcData[0]=0x10082; //request header code
	svcData[1]=regAddr;
	svcData[2]=size;
	svcData[3]=(size<<14)|2;
	svcData[4]=(u32)data;
	svc_sendSyncRequest(handle); //check return value...
}

void GSPGPU_ReadHWRegs(Handle handle, u32 regAddr, u8* data, u8 size)
{
	if(size>0x80 || !data)return;

	u32* svcData=svc_getData();
	svcData[0]=0x40080; //request header code
	svcData[1]=regAddr;
	svcData[2]=size;
	svcData[0x40]=(size<<14)|2;
	svcData[0x40+1]=(u32)data;
	svc_sendSyncRequest(handle); //check return value...
}

void GSPGPU_RegisterInterruptRelayQueue(Handle handle, Handle eventHandle, u32 flags, Handle* outMemHandle, u8* threadID)
{
	u32* svcData=svc_getData();
	svcData[0]=0x130042; //request header code
	svcData[1]=flags;
	svcData[2]=0x0;
	svcData[3]=eventHandle;
	svc_sendSyncRequest(handle); //check return value...
	if(threadID)*threadID=svcData[2];
	if(outMemHandle)*outMemHandle=svcData[4];
}

Result GSPGPU_TriggerCmdReqQueue(Handle handle)
{
	u32* svcData=svc_getData();
	svcData[0]=0xC0000; //request header code
	svc_sendSyncRequest(handle); //check return value...
	return svcData[0];
}

//essentially : get commandIndex and totalCommands, calculate offset of new command, copy command and update totalCommands
//use LDREX/STREX because this data may also be accessed by the GSP module and we don't want to break stuff
//(mostly, we could overwrite the buffer header with wrong data and make the GSP module reexecute old commands)
Result GSPGPU_submitGxCommand(u32* sharedGspCmdBuf, u32 gxCommand[0x8], Handle handle)
{
	if(!sharedGspCmdBuf || !gxCommand)return -1;

	u32 cmdBufHeader;
	__asm__ ("ldrex %[result], [%[adr]]" : [result] "=r" (cmdBufHeader) : [adr] "r" (sharedGspCmdBuf));

	u8 commandIndex=cmdBufHeader&0xFF;
	u8 totalCommands=(cmdBufHeader>>8)&0xFF;

	if(totalCommands>15)return -2;

	u8 nextCmd=(commandIndex+totalCommands)%15; //there are 15 command slots
	u32* dst=&sharedGspCmdBuf[8*(1+nextCmd)];
	memcpy(dst, gxCommand, 0x20);

	u32 mcrVal=0x0;
	__asm__ ("mcr p15, 0, %[val], c7, c10, 4" :: [val] "r" (mcrVal)); //Data Synchronization Barrier Register
	totalCommands++;
	cmdBufHeader=((cmdBufHeader)&0xFFFF00FF)|(((u32)totalCommands)<<8);

	while(1)
	{
		u32 strexResult;
		__asm__ ("strex %[result], %[val], [%[adr]]" : [result] "=r" (strexResult) : [adr] "r" (sharedGspCmdBuf), [val] "r" (cmdBufHeader));
		if(!strexResult)break;

		__asm__ ("ldrex %[result], [%[adr]]" : [result] "=r" (cmdBufHeader) : [adr] "r" (sharedGspCmdBuf));
		totalCommands=((cmdBufHeader&0xFF00)>>8)+1;
		cmdBufHeader=((cmdBufHeader)&0xFFFF00FF)|((totalCommands<<8)|0xFF00);
	}

	if(totalCommands==1)
	{
		GSPGPU_TriggerCmdReqQueue(handle);
	}

	return 0;
}
