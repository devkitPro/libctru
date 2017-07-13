#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/nfc.h>
#include <3ds/services/apt.h>
#include <3ds/ipc.h>

static Handle nfcHandle;
static int nfcRefCount;
static NFC_OpType nfc_optype = NFC_OpType_NFCTag;

static Result NFC_Initialize(NFC_OpType type);
static Result NFC_Shutdown(NFC_OpType type);

static Result NFC_StartCommsAdapter(void);
static Result NFC_StartCommunication(void);
static Result NFC_StopCommunication(void);
static Result NFC_CommunicationGetStatus(u8 *out);
static Result NFC_CommunicationGetResult(Result *out);

static Result NFC_StartTagScanning(u16 unknown);
static Result NFC_StartOtherTagScanning(u16 unk0, u32 unk1);
static Result NFC_StopTagScanning(void);

static Result NFC_InitializeWriteAppData(u32 amiibo_appid, NFC_AppDataInitStruct *initstruct, const void *buf, size_t size);
static Result NFC_GetAppDataInitStruct(NFC_AppDataInitStruct *out);

Result nfcInit(NFC_OpType type)
{
	Result ret=0;

	if (AtomicPostIncrement(&nfcRefCount)) return 0;

	ret = srvGetServiceHandle(&nfcHandle, "nfc:m");
	if (R_FAILED(ret))ret = srvGetServiceHandle(&nfcHandle, "nfc:u");
	if (R_SUCCEEDED(ret))
	{
		nfc_optype = type;
		for (;;)
		{
			ret = NFC_Initialize(type);
			if (ret != 0xd0a17480)
				break;
			svcSleepThread(500000);
		}
		if (R_FAILED(ret)) svcCloseHandle(nfcHandle);
	}
	if (R_FAILED(ret)) AtomicDecrement(&nfcRefCount);

	return ret;
}

void nfcExit(void)
{
	if (AtomicDecrement(&nfcRefCount)) return;
	NFC_Shutdown(nfc_optype);
	svcCloseHandle(nfcHandle);
}

Handle nfcGetSessionHandle(void)
{
	return nfcHandle;
}

static Result NFC_StartCommsAdapter(void)
{
	Result ret, ret2;
	bool new3ds_flag = false;
	u8 status;

	APT_CheckNew3DS(&new3ds_flag);

	if(new3ds_flag) return 0;
	
	ret = NFC_StartCommunication();
	if(R_FAILED(ret))return ret;

	while(1)
	{
		status = 0;
		ret = NFC_CommunicationGetStatus(&status);
		if(R_FAILED(ret))break;

		if(status==1)//"Attempting to initialize Old3DS NFC adapter communication."
		{
			svcSleepThread(1000000*100);
			continue;
		}
		else if(status==2)//"Old3DS NFC adapter communication initialization successfully finished."
		{
			break;
		}

		//An error occured with Old3DS NFC-adapter communication initialization.

		ret = NFC_CommunicationGetResult(&ret2);
		if(R_FAILED(ret))break;

		return ret2;
	}

	return ret;
}

Result nfcStartScanning(u16 inval)
{
	Result ret = NFC_StartCommsAdapter();
	if(R_FAILED(ret)) return ret;

	return NFC_StartTagScanning(inval);
}

Result nfcStartOtherTagScanning(u16 unk0, u32 unk1)
{
	Result ret = NFC_StartCommsAdapter();
	if(R_FAILED(ret)) return ret;
	
	return NFC_StartOtherTagScanning(unk0, unk1);	
}

void nfcStopScanning(void)
{
	bool new3ds_flag = false;

	APT_CheckNew3DS(&new3ds_flag);

	NFC_StopTagScanning();

	if(!new3ds_flag)
	{
		NFC_StopCommunication();
	}
}

static Result NFC_Initialize(NFC_OpType type)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1,1,0); // 0x10040
	cmdbuf[1]=type & 0xff;

	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	return ret;
}

static Result NFC_Shutdown(NFC_OpType type)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x2,1,0); // 0x20040
	cmdbuf[1]=type & 0xff;

	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	return ret;
}

static Result NFC_StartCommunication(void)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x3,0,0); // 0x30000

	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	return ret;
}

static Result NFC_StopCommunication(void)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x4,0,0); // 0x40000

	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	return ret;
}

static Result NFC_StartTagScanning(u16 unknown)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x5,1,0); // 0x50040
	cmdbuf[1]=unknown;

	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	return ret;
}

static Result NFC_StopTagScanning(void)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x6,0,0); // 0x60000

	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	return ret;
}

Result nfcLoadAmiiboData(void)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x7,0,0); // 0x70000

	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	return ret;
}

Result nfcResetTagScanState(void)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x8,0,0); // 0x80000

	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	return ret;
}

Result nfcUpdateStoredAmiiboData(void)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x9,0,2); // 0x90002
	cmdbuf[1]=IPC_Desc_CurProcessHandle();

	if(R_FAILED(ret=svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	return ret;
}

Result nfcGetTagState(NFC_TagState *state)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xD,0,0); // 0xD0000

	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret) && state)*state = cmdbuf[2] & 0xff;

	return ret;
}

static Result NFC_CommunicationGetStatus(u8 *out)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xF,0,0); // 0xF0000

	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret) && out)*out = cmdbuf[2];

	return ret;
}

Result nfcGetTagInfo(NFC_TagInfo *out)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x11,0,0); // 0x110000

	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret) && out)memcpy(out, &cmdbuf[2], sizeof(NFC_TagInfo));

	return ret;
}

static Result NFC_CommunicationGetResult(Result *out)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x12,0,0); // 0x120000

	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret) && out)*out = cmdbuf[2];

	return ret;
}

Result nfcOpenAppData(u32 amiibo_appid)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x13,1,0); // 0x130040
	cmdbuf[1]=amiibo_appid;

	if(R_FAILED(ret=svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	return ret;
}

Result nfcInitializeWriteAppData(u32 amiibo_appid, const void *buf, size_t size)
{
	Result ret=0;
	NFC_AppDataInitStruct initstruct;

	ret = NFC_GetAppDataInitStruct(&initstruct);
	if(R_FAILED(ret))return ret;

	return NFC_InitializeWriteAppData(amiibo_appid, &initstruct, buf, size);
}

static Result NFC_InitializeWriteAppData(u32 amiibo_appid, NFC_AppDataInitStruct *initstruct, const void *buf, size_t size)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x14,14,4); // 0x140384
	cmdbuf[1]=amiibo_appid;
	cmdbuf[2]=size;
	memcpy(&cmdbuf[3], initstruct->data_xc, sizeof(initstruct->data_xc));
	cmdbuf[15]=IPC_Desc_CurProcessHandle();
	cmdbuf[17]=IPC_Desc_StaticBuffer(size, 0);
	cmdbuf[18]=(u32)buf;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(nfcHandle)))return ret;

	return cmdbuf[1];
}

Result nfcReadAppData(void *buf, size_t size)
{
	u32* cmdbuf=getThreadCommandBuffer();
	u32 saved_threadstorage[2];

	cmdbuf[0]=IPC_MakeHeader(0x15,1,0); // 0x150040
	cmdbuf[1]=size;

	u32 * staticbufs = getThreadStaticBuffers();
	saved_threadstorage[0] = staticbufs[0];
	saved_threadstorage[1] = staticbufs[1];

	staticbufs[0] = IPC_Desc_StaticBuffer(size,0);
	staticbufs[1] = (u32)buf;

	Result ret=0;
	ret=svcSendSyncRequest(nfcHandle);

	staticbufs[0] = saved_threadstorage[0];
	staticbufs[1] = saved_threadstorage[1];

	if(R_FAILED(ret))return ret;

	ret = cmdbuf[1];

	return ret;
}

Result nfcWriteAppData(const void *buf, size_t size, NFC_TagInfo *taginfo)
{
	u32* cmdbuf=getThreadCommandBuffer();
	NFC_AppDataWriteStruct writestruct;

	cmdbuf[0]=IPC_MakeHeader(0x16,9,2); // 0x160242
	cmdbuf[1]=size;
	cmdbuf[10]=IPC_Desc_StaticBuffer(size, 0);
	cmdbuf[11]=(u32)buf;

	if(taginfo==NULL)return -1;
	if(taginfo->id_offset_size>10)return -2;

	memset(&writestruct, 0, sizeof(NFC_AppDataWriteStruct));
	writestruct.id_size = taginfo->id_offset_size;
	memcpy(writestruct.id, taginfo->id, sizeof(writestruct.id));

	memcpy(&cmdbuf[2], &writestruct, sizeof(NFC_AppDataWriteStruct));

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(nfcHandle)))return ret;

	return cmdbuf[1];
}

Result nfcGetAmiiboSettings(NFC_AmiiboSettings *out)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x17,0,0); // 0x170000

	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret) && out)memcpy(out, &cmdbuf[2], sizeof(NFC_AmiiboSettings));

	return ret;
}

Result nfcGetAmiiboConfig(NFC_AmiiboConfig *out)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x18,0,0); // 0x180000

	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret) && out)memcpy(out, &cmdbuf[2], sizeof(NFC_AmiiboConfig));

	return ret;
}

static Result NFC_GetAppDataInitStruct(NFC_AppDataInitStruct *out)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x19,0,0); // 0x190000

	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret) && out)memcpy(out, &cmdbuf[2], sizeof(NFC_AppDataInitStruct));

	return ret;
}

static Result NFC_StartOtherTagScanning(u16 unk0, u32 unk1)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1F,2,0); // 0x1F0080
	cmdbuf[1] = unk0;
	cmdbuf[2] = unk1;

	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	return ret;
}

Result nfcSendTagCommand(const void *inbuf, size_t insize, void *outbuf, size_t outsize, size_t *actual_transfer_size, u64 microseconds)
{
	u32* cmdbuf=getThreadCommandBuffer();
	u32 saved_threadstorage[2];

	cmdbuf[0]=IPC_MakeHeader(0x20,4,2); // 0x200102
	cmdbuf[1]=insize;
	cmdbuf[2]=outsize;
	cmdbuf[3]=(u32)(microseconds);
	cmdbuf[4]=(u32)(microseconds>>32);
	cmdbuf[5]=IPC_Desc_StaticBuffer(insize,0);
	cmdbuf[6]=(u32)inbuf;

	u32 * staticbufs = getThreadStaticBuffers();
	saved_threadstorage[0] = staticbufs[0];
	saved_threadstorage[1] = staticbufs[1];

	staticbufs[0] = IPC_Desc_StaticBuffer(outsize,0);
	staticbufs[1] = (u32)outbuf;

	Result ret=0;
	ret=svcSendSyncRequest(nfcHandle);

	staticbufs[0] = saved_threadstorage[0];
	staticbufs[1] = saved_threadstorage[1];

	if(R_FAILED(ret))return ret;

	ret = cmdbuf[1];
	if(actual_transfer_size)*actual_transfer_size = cmdbuf[2];

	return ret;
}

Result nfcCmd21(void)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x21,0,0); // 0x210000

	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	return ret;
}

Result nfcCmd22(void)
{
	Result ret=0;
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x22,0,0); // 0x220000

	if(R_FAILED(ret = svcSendSyncRequest(nfcHandle)))return ret;
	ret = cmdbuf[1];

	return ret;
}

