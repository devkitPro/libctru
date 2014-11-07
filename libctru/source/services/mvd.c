/*
  mvd.c - code for using this: http://3dbrew.org/wiki/MVD_Services
*/

#include <stdlib.h>
#include <string.h>
#include <3ds.h>

Handle mvdstdHandle;
static u32 mvdstdInitialized = 0;
static mvdstdType mvdstd_type;
static u32 *mvdstd_workbuf = NULL;
static size_t mvdstd_workbufsize = 0;

static Result mvdstdipc_Initialize(u32 *buf, u32 bufsize, Handle kprocess)
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00010082; //request header code
	cmdbuf[1] = (u32)buf;
	cmdbuf[2] = bufsize;
	cmdbuf[3] = 0;
	cmdbuf[4] = kprocess;

	Result ret=0;
	if((ret=svcSendSyncRequest(mvdstdHandle)))return ret;

	return cmdbuf[1];
}

static Result mvdstdipc_Shutdown()
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00020000; //request header code

	Result ret=0;
	if((ret = svcSendSyncRequest(mvdstdHandle)))return ret;

	return cmdbuf[1];
}

static Result mvdstdipc_cmd18()
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00180000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(mvdstdHandle)))return ret;

	return cmdbuf[1];
}

static Result mvdstdipc_cmd19()
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00190000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(mvdstdHandle)))return ret;

	return cmdbuf[1];
}

static Result mvdstdipc_cmd1a()
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x001A0000; //request header code

	Result ret=0;
	if((ret=svcSendSyncRequest(mvdstdHandle)))return ret;

	return cmdbuf[1];
}

Result mvdstdSetConfig(mvdstdConfig *config)
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x001E0044; //request header code
	cmdbuf[1] = sizeof(mvdstdConfig);
	cmdbuf[2] = 0;
	cmdbuf[3] = 0xffff8001;
	cmdbuf[4] = (sizeof(mvdstdConfig)<<4) | 10;
	cmdbuf[5] = (u32)config;

	Result ret=0;
	if((ret=svcSendSyncRequest(mvdstdHandle)))return ret;

	return cmdbuf[1];
}

void mvdstdGenerateDefaultConfig(mvdstdConfig *config, u32 width, u32 height, u32 *vaddr_colorconv_indata, u32 *vaddr_outdata0, u32 *vaddr_outdata1_colorconv)
{
	memset(config, 0, sizeof(mvdstdConfig));

	config->type = mvdstd_type;

	config->width0 = width;
	config->height0 = height;

	if(mvdstd_type==MVDTYPE_COLORFORMATCONV)config->physaddr_colorconv_indata = osConvertVirtToPhys((u32)vaddr_colorconv_indata);

	if(mvdstd_type==MVDTYPE_H264)
	{
		config->flag_x40 = 1;
		config->height1 = height;
		config->width1 = width;
	}

	config->unk_x58 = 0x40002;

	config->width2 = width;
	config->height2 = height;

	config->physaddr_outdata0 = osConvertVirtToPhys((u32)vaddr_outdata0);
	if(mvdstd_type==MVDTYPE_COLORFORMATCONV)config->physaddr_outdata1_colorconv = osConvertVirtToPhys((u32)vaddr_outdata1_colorconv);

	config->unk_x6c[0] = 0x1;
	config->unk_x6c[(0x84-0x6c)>>2] = 0x12a;
	config->unk_x6c[(0x88-0x6c)>>2] = 0x199;
	config->unk_x6c[(0x8c-0x6c)>>2] = 0xd0;
	config->unk_x6c[(0x90-0x6c)>>2] = 0x64;
	config->unk_x6c[(0x94-0x6c)>>2] = 0x204;
	config->unk_x6c[(0xa8-0x6c)>>2] = 0x1;
	config->unk_x6c[(0x104-0x6c)>>2] = 0x1;
	config->unk_x6c[(0x110-0x6c)>>2] = 0x200;
	config->unk_x6c[(0x114-0x6c)>>2] = 0x100;
}

Result mvdstdInit(mvdstdType type, u32 size)
{
	Result ret=0;

	if(mvdstdInitialized)return 0;

	mvdstd_workbufsize = size;
	mvdstd_type = type;

	if(type==MVDTYPE_COLORFORMATCONV)mvdstd_workbufsize = 1;
	if(type!=MVDTYPE_COLORFORMATCONV)return -2;//H264 isn't supported atm.

	if((ret=srvGetServiceHandle(&mvdstdHandle, "mvd:STD")))return ret;

	mvdstd_workbuf = linearAlloc(mvdstd_workbufsize);
	if(mvdstd_workbuf==NULL)return -1;

	ret = mvdstdipc_Initialize((u32*)osConvertOldLINEARMemToNew((u32)mvdstd_workbuf), mvdstd_workbufsize, 0xffff8001);
	if(ret<0)
	{
		svcCloseHandle(mvdstdHandle);
		linearFree(mvdstd_workbuf);
		return ret;
	}

	ret = mvdstdipc_cmd18();
	if(ret<0)
	{
		mvdstdipc_Shutdown();
		svcCloseHandle(mvdstdHandle);
		linearFree(mvdstd_workbuf);
		return ret;
	}

	mvdstdInitialized = 1;

	return 0;
}

Result mvdstdShutdown()
{
	if(!mvdstdInitialized)return 0;

	if(mvdstd_type==MVDTYPE_COLORFORMATCONV)
	{
		mvdstdipc_cmd19();
	}

	mvdstdipc_Shutdown();

	svcCloseHandle(mvdstdHandle);

	linearFree(mvdstd_workbuf);

	return 0;
}

Result mvdstdProcessFrame(mvdstdConfig *config, u32 *h264_vaddr_inframe, u32 h264_inframesize, u32 h264_frameid)
{
	Result ret;

	if(config==NULL)return -1;
	if(mvdstd_type!=MVDTYPE_COLORFORMATCONV)return -2;

	ret = mvdstdSetConfig(config);
	if(ret<0)return ret;

	return mvdstdipc_cmd1a();
}

