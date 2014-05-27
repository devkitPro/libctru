#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/FS.h>
#include <ctr/svc.h>

Result FSUSER_Initialize(Handle handle)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x08010002; //request header code
	cmdbuf[1]=32;
	
	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;
	
	return cmdbuf[1];
}

Result FSUSER_OpenFile(Handle handle, Handle* out, FS_archive archive, FS_path fileLowPath, u32 openflags, u32 attributes) //archive needs to have been opened
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=0x080201C2;
	cmdbuf[1]=0;
	cmdbuf[2]=archive.handleLow;
	cmdbuf[3]=archive.handleHigh;
	cmdbuf[4]=fileLowPath.type;
	cmdbuf[5]=fileLowPath.size;
	cmdbuf[6]=openflags;
	cmdbuf[7]=attributes;
	cmdbuf[8]=(fileLowPath.size<<14)|2;
	cmdbuf[9]=(u32)fileLowPath.data;
 
	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;
 
	if(out)*out=cmdbuf[3];
 
	return cmdbuf[1];
}

Result FSUSER_OpenFileDirectly(Handle handle, Handle* out, FS_archive archive, FS_path fileLowPath, u32 openflags, u32 attributes) //no need to have archive opened
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=0x08030204;
	cmdbuf[1]=0;
	cmdbuf[2]=archive.id;
	cmdbuf[3]=archive.lowPath.type;
	cmdbuf[4]=archive.lowPath.size;
	cmdbuf[5]=fileLowPath.type;
	cmdbuf[6]=fileLowPath.size;
	cmdbuf[7]=openflags;
	cmdbuf[8]=attributes;
	cmdbuf[9]=(archive.lowPath.size<<14)|0x802;
	cmdbuf[10]=(u32)archive.lowPath.data;
	cmdbuf[11]=(fileLowPath.size<<14)|2;
	cmdbuf[12]=(u32)fileLowPath.data;
 
	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;
 
	if(out)*out=cmdbuf[3];
 
	return cmdbuf[1];
}

Result FSUSER_OpenArchive(Handle handle, FS_archive* archive)
{
	if(!archive)return -2;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=0x080C00C2;
	cmdbuf[1]=archive->id;
	cmdbuf[2]=archive->lowPath.type;
	cmdbuf[3]=archive->lowPath.size;
	cmdbuf[4]=(archive->lowPath.size<<14)|0x2;
	cmdbuf[5]=(u32)archive->lowPath.data;
 
	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;
 
	archive->handleLow=cmdbuf[2];
	archive->handleHigh=cmdbuf[3];
 
	return cmdbuf[1];
}

Result FSUSER_OpenDirectory(Handle handle, Handle* out, FS_archive archive, FS_path dirLowPath)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=0x080B0102;
	cmdbuf[1]=archive.handleLow;
	cmdbuf[2]=archive.handleHigh;
	cmdbuf[3]=dirLowPath.type;
	cmdbuf[4]=dirLowPath.size;
	cmdbuf[5]=(dirLowPath.size<<14)|0x2;
	cmdbuf[6]=(u32)dirLowPath.data;
 
	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;
 
	if(out)*out=cmdbuf[3];
 
	return cmdbuf[1];
}

Result FSUSER_CloseArchive(Handle handle, FS_archive* archive)
{
	if(!archive)return -2;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=0x080E0080;
	cmdbuf[1]=archive->handleLow;
	cmdbuf[2]=archive->handleLow;
 
	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;
 
	return cmdbuf[1];
}

Result FSFILE_Close(Handle handle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=0x08080000;

	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

Result FSFILE_Read(Handle handle, u32 *bytesRead, u64 offset, u32 *buffer, u32 size)
{
	u32 *cmdbuf=getThreadCommandBuffer();
 
	cmdbuf[0]=0x080200C2;
	cmdbuf[1]=(u32)offset;
	cmdbuf[2]=(u32)(offset>>32);
	cmdbuf[3]=size;
	cmdbuf[4]=(size<<4)|12;
	cmdbuf[5]=(u32)buffer;
 
	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;

	if(bytesRead)*bytesRead=cmdbuf[2];

	return cmdbuf[1];
}

//WARNING : using wrong flushFlags CAN corrupt the archive you're writing to.
//another warning : data should *not* be in RO memory
Result FSFILE_Write(Handle handle, u32 *bytesWritten, u64 offset, u32 *data, u32 size, u32 flushFlags)
{
	u32 *cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=0x08030102;
	cmdbuf[1]=(u32)offset;
	cmdbuf[2]=(u32)(offset>>32);
	cmdbuf[3]=size;
	cmdbuf[4]=flushFlags;
	cmdbuf[5]=(size<<4)|10;
	cmdbuf[6]=(u32)data;

	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;

	if(bytesWritten)*bytesWritten=cmdbuf[2];

	return cmdbuf[1];
}

Result FSFILE_GetSize(Handle handle, u64 *size)
{
	u32 *cmdbuf=getThreadCommandBuffer();
 
	cmdbuf[0] = 0x08040000;
 
	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;
 
	if(size)*size = *((u64*)&cmdbuf[2]);
 
	return cmdbuf[1];
}

Result FSFILE_SetSize(Handle handle, u64 size)
{
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = 0x08050080;
    cmdbuf[1] = (u32)size;
    cmdbuf[2] = (u32)(size >> 32);

    Result ret = 0;
    if ((ret = svc_sendSyncRequest(handle)))return ret;


    return cmdbuf[1];
}

Result FSDIR_Read(Handle handle, u32 *entriesRead, u32 entrycount, u16 *buffer)
{
	u32 *cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=0x08010042;
	cmdbuf[1]=entrycount;
	cmdbuf[2]=((entrycount*0x228)<<4)|0xC;
	cmdbuf[3]=(u32)buffer;

	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;

	if(entriesRead)*entriesRead=cmdbuf[2];

	return cmdbuf[1];
}

Result FSDIR_Close(Handle handle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=0x08020000;

	Result ret=0;
	if((ret=svc_sendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}
