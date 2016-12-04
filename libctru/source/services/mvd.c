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

static u32 mvdstd_videoproc_frameid;

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

static Result MVDSTD_cmd5(s8 unk0, s8 unk1, s8 unk2, u32 unk3)
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x5,4,0); // 0x50100
	cmdbuf[1] = unk0;
	cmdbuf[2] = unk1;
	cmdbuf[3] = unk2;
	cmdbuf[4] = unk3;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(mvdstdHandle)))return ret;

	return cmdbuf[1];
}

static Result MVDSTD_cmd7(void)
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x7,0,0); // 0x70000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(mvdstdHandle)))return ret;

	return cmdbuf[1];
}

static Result MVDSTD_ProcessNALUnit(u32 vaddr_buf, u32 physaddr_buf, u32 size, u32 frameid, u32 flag, MVDSTD_ProcessNALUnitOut *out)
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x8,5,2); // 0x80142
	cmdbuf[1] = vaddr_buf;
	cmdbuf[2] = physaddr_buf;
	cmdbuf[3] = size;
	cmdbuf[4] = frameid;
	cmdbuf[5] = flag;
	cmdbuf[6] = IPC_Desc_SharedHandles(1);
	cmdbuf[7] = CUR_PROCESS_HANDLE;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(mvdstdHandle)))return ret;

	if(out)memcpy(out, &cmdbuf[2], sizeof(MVDSTD_ProcessNALUnitOut));

	return cmdbuf[1];
}

static Result MVDSTD_ControlFrameRendering(s8 type)
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x9,1,2); // 0x90042
	cmdbuf[1] = type;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = CUR_PROCESS_HANDLE;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(mvdstdHandle)))return ret;

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

static Result MVDSTD_cmd1b(u8 unk)
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1B,1,0); // 0x1B0040
	cmdbuf[1] = unk;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(mvdstdHandle)))return ret;

	return cmdbuf[1];
}

static Result MVDSTD_cmd1c(void)
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1C,0,0); // 0x1C0000

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

Result mvdstdSetupOutputBuffers(MVDSTD_OutputBuffersEntryList *entrylist, u32 bufsize)
{
	Result ret=0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1F,36,2); // 0x1F0902
	memcpy(&cmdbuf[1], entrylist, sizeof(MVDSTD_OutputBuffersEntryList));
	cmdbuf[36] = bufsize;
	cmdbuf[37] = IPC_Desc_SharedHandles(1);
	cmdbuf[38] = CUR_PROCESS_HANDLE;

	if(R_FAILED(ret=svcSendSyncRequest(mvdstdHandle)))return ret;

	return cmdbuf[1];
}

Result mvdstdOverrideOutputBuffers(void* cur_outdata0, void* cur_outdata1, void* new_outdata0, void* new_outdata1)
{
	Result ret=0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x21,4,0); // 0x210100
	cmdbuf[1] = (u32)cur_outdata0;
	cmdbuf[2] = (u32)cur_outdata1;
	cmdbuf[3] = (u32)new_outdata0;
	cmdbuf[4] = (u32)new_outdata1;

	if(R_FAILED(ret=svcSendSyncRequest(mvdstdHandle)))return ret;

	return cmdbuf[1];
}

Result mvdstdInit(MVDSTD_Mode mode, MVDSTD_InputFormat input_type, MVDSTD_OutputFormat output_type, u32 size, MVDSTD_InitStruct *initstruct)
{
	Result ret=0, ret2=0;

	mvdstd_workbufsize = size;
	mvdstd_mode = mode;
	mvdstd_input_type = input_type;
	mvdstd_output_type = output_type;

	mvdstd_videoproc_frameid = 0;

	MVDSTD_InitStruct tmpinitstruct;

	if(mvdstd_mode==MVDMODE_COLORFORMATCONV)mvdstd_workbufsize = 1;

	if (AtomicPostIncrement(&mvdstdRefCount)) return 0;

	memset(&tmpinitstruct, 0, sizeof(MVDSTD_InitStruct));
	tmpinitstruct.cmd1b_inval = 1;
	if(initstruct)memcpy(&tmpinitstruct, initstruct, sizeof(MVDSTD_InitStruct));

	if(R_FAILED(ret=srvGetServiceHandle(&mvdstdHandle, "mvd:STD"))) goto cleanup0;

	mvdstd_workbuf = linearAlloc(mvdstd_workbufsize);
	if(mvdstd_workbuf==NULL)
	{
		ret = -1;
		goto cleanup1;
	}
	memset(mvdstd_workbuf, 0, mvdstd_workbufsize);

	ret = MVDSTD_Initialize((u32*) osConvertOldLINEARMemToNew(mvdstd_workbuf), mvdstd_workbufsize);
	if(R_FAILED(ret)) goto cleanup2;

	if(mvdstd_mode==MVDMODE_VIDEOPROCESSING)
	{
		ret = MVDSTD_cmd5(tmpinitstruct.cmd5_inval0, tmpinitstruct.cmd5_inval1, tmpinitstruct.cmd5_inval2, tmpinitstruct.cmd5_inval3);
		if(ret!=MVD_STATUS_OK) goto cleanup3;
	}

	ret = MVDSTD_cmd18();
	if(ret!=MVD_STATUS_OK) goto cleanup3;

	if(mvdstd_mode==MVDMODE_VIDEOPROCESSING)
	{
		ret = MVDSTD_cmd1b(tmpinitstruct.cmd1b_inval);
		if(ret!=MVD_STATUS_OK) goto cleanup3;
	}

	return 0;

cleanup3:
	ret2 = MVD_STATUS_BUSY;
	while(ret2==MVD_STATUS_BUSY)ret2 = MVDSTD_ControlFrameRendering(1);

	if(mvdstd_mode==MVDMODE_VIDEOPROCESSING)MVDSTD_cmd1c();

	MVDSTD_cmd19();

	if(mvdstd_mode==MVDMODE_VIDEOPROCESSING)MVDSTD_cmd7();

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
	Result ret=0;

	if (AtomicDecrement(&mvdstdRefCount)) return;

	ret = MVD_STATUS_BUSY;
	while(ret==MVD_STATUS_BUSY)ret = MVDSTD_ControlFrameRendering(1);

	if(mvdstd_mode==MVDMODE_VIDEOPROCESSING)MVDSTD_cmd1c();

	MVDSTD_cmd19();

	if(mvdstd_mode==MVDMODE_VIDEOPROCESSING)MVDSTD_cmd7();

	MVDSTD_Shutdown();

	svcCloseHandle(mvdstdHandle);

	linearFree(mvdstd_workbuf);
}

void mvdstdGenerateDefaultConfig(MVDSTD_Config*config, u32 input_width, u32 input_height, u32 output_width, u32 output_height, u32 *vaddr_colorconv_indata, u32 *vaddr_outdata0, u32 *vaddr_outdata1)
{
	memset(config, 0, sizeof(MVDSTD_Config));

	config->input_type = mvdstd_input_type;

	config->inwidth = input_width;
	config->inheight = input_height;

	if(mvdstd_mode==MVDMODE_COLORFORMATCONV)config->physaddr_colorconv_indata = osConvertVirtToPhys(vaddr_colorconv_indata);

	config->output_type = mvdstd_output_type;

	config->outwidth = output_width;
	config->outheight = output_height;

	config->physaddr_outdata0 = osConvertVirtToPhys(vaddr_outdata0);
	if(config->output_type==0x00020001)config->physaddr_outdata1 = osConvertVirtToPhys(vaddr_outdata1);

	config->unk_x6c[0] = 0x1;
	config->unk_x6c[(0x84-0x6c)>>2] = 0x12a;
	config->unk_x6c[(0x88-0x6c)>>2] = 0x199;
	config->unk_x6c[(0x8c-0x6c)>>2] = 0xd0;
	config->unk_x6c[(0x90-0x6c)>>2] = 0x64;
	config->unk_x6c[(0x94-0x6c)>>2] = 0x204;
	config->unk_x6c[(0xa8-0x6c)>>2] = 0x1;
}

Result mvdstdConvertImage(MVDSTD_Config* config)
{
	Result ret;

	if(mvdstdRefCount==0)return -3;
	if(config==NULL)return -1;
	if(mvdstd_mode!=MVDMODE_COLORFORMATCONV)return -2;

	ret = MVDSTD_SetConfig(config);
	if(ret!=MVD_STATUS_OK)return ret;

	return MVDSTD_cmd1a();
}

Result mvdstdProcessVideoFrame(void* inbuf_vaddr, size_t size, u32 flag, MVDSTD_ProcessNALUnitOut *out)
{
	Result ret;

	if(mvdstdRefCount==0)return -3;
	if(mvdstd_mode!=MVDMODE_VIDEOPROCESSING)return -2;

	ret = MVDSTD_ProcessNALUnit((u32)inbuf_vaddr, (u32)osConvertVirtToPhys(inbuf_vaddr), size, mvdstd_videoproc_frameid, flag, out);
	mvdstd_videoproc_frameid++;
	if(mvdstd_videoproc_frameid>=0x12)mvdstd_videoproc_frameid = 0;

	return ret;
}

Result mvdstdRenderVideoFrame(MVDSTD_Config* config, bool wait)
{
	Result ret;

	if(mvdstdRefCount==0)return -3;
	if(config==NULL)return -1;
	if(mvdstd_mode!=MVDMODE_VIDEOPROCESSING)return -2;

	if(config)
	{
		ret = MVDSTD_SetConfig(config);
		if(ret!=MVD_STATUS_OK)return ret;
	}

	ret = MVD_STATUS_BUSY;
	while(ret==MVD_STATUS_BUSY)
	{
		ret = MVDSTD_ControlFrameRendering(0);
		if(!wait)break;
	}

	return ret;
}

