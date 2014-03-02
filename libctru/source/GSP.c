#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/GSP.h>
#include <ctr/svc.h>
#include <ctr/srv.h>

Handle gspGpuHandle=0;

void gspInit()
{
	//do stuff with GPU...
	srv_getServiceHandle(NULL, &gspGpuHandle, "gsp::Gpu");
}

void gspExit()
{
	if(gspGpuHandle)svc_closeHandle(gspGpuHandle);
}

Result GSPGPU_AcquireRight(Handle* handle, u8 flags)
{
	if(!handle)handle=&gspGpuHandle;
	
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x160042; //request header code
	cmdbuf[1]=flags;
	cmdbuf[2]=0x0;
	cmdbuf[3]=0xffff8001;

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_ReleaseRight(Handle* handle)
{
	if(!handle)handle=&gspGpuHandle;
	
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x170000; //request header code

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_SetLcdForceBlack(Handle* handle, u8 flags)
{
	if(!handle)handle=&gspGpuHandle;
	
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0xB0040; //request header code
	cmdbuf[1]=flags;

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_FlushDataCache(Handle* handle, u8* adr, u32 size)
{
	if(!handle)handle=&gspGpuHandle;
	
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x80082; //request header code
	cmdbuf[1]=(u32)adr;
	cmdbuf[2]=size;
	cmdbuf[3]=0x0;
	cmdbuf[4]=0xffff8001;

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_WriteHWRegs(Handle* handle, u32 regAddr, u32* data, u8 size)
{
	if(!handle)handle=&gspGpuHandle;
	
	if(size>0x80 || !data)return -1;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x10082; //request header code
	cmdbuf[1]=regAddr;
	cmdbuf[2]=size;
	cmdbuf[3]=(size<<14)|2;
	cmdbuf[4]=(u32)data;

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_WriteHWRegsWithMask(Handle* handle, u32 regAddr, u32* data, u8 datasize, u32* maskdata, u8 masksize)
{
	if(!handle)handle=&gspGpuHandle;
	
	if(datasize>0x80 || !data)return -1;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x20084; //request header code
	cmdbuf[1]=regAddr;
	cmdbuf[2]=datasize;
	cmdbuf[3]=(datasize<<14)|2;
	cmdbuf[4]=(u32)data;
	cmdbuf[5]=(masksize<<14)|0x402;
	cmdbuf[6]=(u32)maskdata;

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_ReadHWRegs(Handle* handle, u32 regAddr, u32* data, u8 size)
{
	if(!handle)handle=&gspGpuHandle;
	
	if(size>0x80 || !data)return -1;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x40080; //request header code
	cmdbuf[1]=regAddr;
	cmdbuf[2]=size;
	cmdbuf[0x40]=(size<<14)|2;
	cmdbuf[0x40+1]=(u32)data;

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_RegisterInterruptRelayQueue(Handle* handle, Handle eventHandle, u32 flags, Handle* outMemHandle, u8* threadID)
{
	if(!handle)handle=&gspGpuHandle;
	
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x130042; //request header code
	cmdbuf[1]=flags;
	cmdbuf[2]=0x0;
	cmdbuf[3]=eventHandle;

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	if(threadID)*threadID=cmdbuf[2];
	if(outMemHandle)*outMemHandle=cmdbuf[4];
	
	return cmdbuf[1];
}

Result GSPGPU_UnregisterInterruptRelayQueue(Handle* handle)
{
	if(!handle)handle=&gspGpuHandle;
	
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00140000; //request header code

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;
	
	return cmdbuf[1];
}

Result GSPGPU_TriggerCmdReqQueue(Handle* handle)
{
	if(!handle)handle=&gspGpuHandle;
	
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0xC0000; //request header code

	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

//essentially : get commandIndex and totalCommands, calculate offset of new command, copy command and update totalCommands
//use LDREX/STREX because this data may also be accessed by the GSP module and we don't want to break stuff
//(mostly, we could overwrite the buffer header with wrong data and make the GSP module reexecute old commands)
Result GSPGPU_submitGxCommand(u32* sharedGspCmdBuf, u32 gxCommand[0x8], Handle* handle)
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
		__asm__ ("strex %[result], %[val], [%[adr]]" : [result] "=&r" (strexResult) : [adr] "r" (sharedGspCmdBuf), [val] "r" (cmdBufHeader));
		if(!strexResult)break;

		__asm__ ("ldrex %[result], [%[adr]]" : [result] "=r" (cmdBufHeader) : [adr] "r" (sharedGspCmdBuf));
		totalCommands=((cmdBufHeader&0xFF00)>>8)+1;
		cmdBufHeader=((cmdBufHeader)&0xFFFF00FF)|((totalCommands<<8)|0xFF00);
	}

	if(totalCommands==1)return GSPGPU_TriggerCmdReqQueue(handle);
	return 0;
}
