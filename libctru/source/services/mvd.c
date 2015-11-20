/*
  mvd.c - code for using this: http://3dbrew.org/wiki/MVD_Services
*/

#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/os.h>
#include <3ds/allocator/linear.h>
#include <3ds/synchronization.h>
#include <3ds/services/mvd.h>
#include <3ds/ipc.h>

Handle mvdstdHandle;
static int mvdstdRefCount;
static MVDSTD_Mode mvdstd_mode;
static MVDSTD_InputFormat mvdstd_input_type;
static MVDSTD_OutputFormat mvdstd_output_type;
static u32 *mvdstd_workbuf;
static size_t mvdstd_workbufsize;

static Result MVDSTD_Initialize(u32* buf, u32 bufsize)
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1,2,2); // 0x10082
	cmdbuf[1] = (u32)buf;
	cmdbuf[2] = bufsize;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = CUR_PROCESS_HANDLE;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(mvdstdHandle)))return ret;

	return cmdbuf[1];
}

static Result MVDSTD_Shutdown(void)
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2,0,0); // 0x20000

	Result ret=0;
	if((ret = svcSendSyncRequest(mvdstdHandle)))return ret;

	return cmdbuf[1];
}

static Result MVDSTD_cmd18(void)
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x18,0,0); // 0x180000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(mvdstdHandle)))return ret;

	return cmdbuf[1];
}

static Result MVDSTD_cmd19(void)
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x19,0,0); // 0x190000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(mvdstdHandle)))return ret;

	return cmdbuf[1];
}

static Result MVDSTD_cmd1a(void)
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1A,0,0); // 0x1A0000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(mvdstdHandle)))return ret;

	return cmdbuf[1];
}

Result MVDSTD_SetConfig(MVDSTD_Config* config)
{
	Result ret=0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1E,1,4); // 0x1E0044
	cmdbuf[1] = sizeof(MVDSTD_Config);
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = CUR_PROCESS_HANDLE;
	cmdbuf[4] = IPC_Desc_Buffer(sizeof(MVDSTD_Config),IPC_BUFFER_R);
	cmdbuf[5] = (u32)config;

	if(R_FAILED(ret=svcSendSyncRequest(mvdstdHandle)))return ret;

	return cmdbuf[1];
}

Result mvdstdInit(MVDSTD_Mode mode, MVDSTD_InputFormat input_type, MVDSTD_OutputFormat output_type, u32 size)
{
	Result ret=0;

	mvdstd_workbufsize = size;
	mvdstd_mode = mode;
	mvdstd_input_type = input_type;
	mvdstd_output_type = output_type;

	if(mvdstd_mode==MVDMODE_COLORFORMATCONV)mvdstd_workbufsize = 1;
	if(mvdstd_mode!=MVDMODE_COLORFORMATCONV)return -2;//Video processing / H.264 isn't supported atm.

	if (AtomicPostIncrement(&mvdstdRefCount)) return 0;

	if(R_FAILED(ret=srvGetServiceHandle(&mvdstdHandle, "mvd:STD"))) goto cleanup0;

	mvdstd_workbuf = linearAlloc(mvdstd_workbufsize);
	if(mvdstd_workbuf==NULL) goto cleanup1;

	ret = MVDSTD_Initialize((u32*) osConvertOldLINEARMemToNew(mvdstd_workbuf), mvdstd_workbufsize);
	if(R_FAILED(ret)) goto cleanup2;

	ret = MVDSTD_cmd18();
	if(R_FAILED(ret)) goto cleanup3;

	return ret;

cleanup3:
	MVDSTD_Shutdown();
cleanup2:
	linearFree(mvdstd_workbuf);
cleanup1:
	svcCloseHandle(mvdstdHandle);
cleanup0:
	AtomicDecrement(&mvdstdRefCount);
	return ret;
}

void mvdstdExit(void)
{
	if (AtomicDecrement(&mvdstdRefCount)) return;

	if(mvdstd_mode==MVDMODE_COLORFORMATCONV)
	{
		MVDSTD_cmd19();
	}

	MVDSTD_Shutdown();

	svcCloseHandle(mvdstdHandle);

	linearFree(mvdstd_workbuf);
}

void mvdstdGenerateDefaultConfig(MVDSTD_Config*config, u32 input_width, u32 input_height, u32 output_width, u32 output_height, u32 *vaddr_colorconv_indata, u32 *vaddr_outdata0, u32 *vaddr_outdata1_colorconv)
{
	memset(config, 0, sizeof(MVDSTD_Config));

	config->input_type = mvdstd_input_type;

	config->inwidth = input_width;
	config->inheight = input_height;

	if(mvdstd_mode==MVDMODE_COLORFORMATCONV)config->physaddr_colorconv_indata = osConvertVirtToPhys(vaddr_colorconv_indata);

	if(mvdstd_mode==MVDMODE_VIDEOPROCESSING)
	{
		config->flag_x40 = 1;
		config->outheight0 = output_height;
		config->outwidth0 = output_width;
	}

	config->output_type = mvdstd_output_type;

	config->outwidth1 = output_width;
	config->outheight1 = output_height;

	config->physaddr_outdata0 = osConvertVirtToPhys(vaddr_outdata0);
	if(mvdstd_mode==MVDMODE_COLORFORMATCONV)config->physaddr_outdata1_colorconv = osConvertVirtToPhys(vaddr_outdata1_colorconv);

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

Result mvdstdProcessFrame(MVDSTD_Config*config, u32 *h264_vaddr_inframe, u32 h264_inframesize, u32 h264_frameid)
{
	Result ret;

	if(mvdstdRefCount==0)return 0;
	if(config==NULL)return -1;
	if(mvdstd_mode!=MVDMODE_COLORFORMATCONV)return -2;

	ret = MVDSTD_SetConfig(config);
	if(R_FAILED(ret))return ret;

	return MVDSTD_cmd1a();
}

