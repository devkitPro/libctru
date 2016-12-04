#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/sslc.h>
#include <3ds/ipc.h>

#include "soc/soc_common.h"

Handle __sslc_servhandle;
static int __sslc_refcount;

static Result sslcipc_Initialize(void);

Result sslcInit(Handle session_handle)
{
	Result ret=0;

	if (AtomicPostIncrement(&__sslc_refcount)) return 0;

	__sslc_servhandle = session_handle;

	if(__sslc_servhandle==0)ret = srvGetServiceHandle(&__sslc_servhandle, "ssl:C");
	if(session_handle==0 && R_SUCCEEDED(ret))ret = sslcipc_Initialize();
	if (R_FAILED(ret)) AtomicDecrement(&__sslc_refcount);

	return ret;
}

void sslcExit(void)
{
	if (AtomicDecrement(&__sslc_refcount)) return;

	svcCloseHandle(__sslc_servhandle);
	__sslc_servhandle = 0;
}

static Result sslcipc_Initialize(void)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1,0,2); // 0x10002
	cmdbuf[1]=IPC_Desc_CurProcessHandle();

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__sslc_servhandle)))return ret;

	return cmdbuf[1];
}

static Result sslcipc_CreateContext(sslcContext *context, int sockfd, u32 input_opt, const char *hostname)
{
	u32* cmdbuf=getThreadCommandBuffer();
	u32 size = strlen(hostname)+1;

	cmdbuf[0]=IPC_MakeHeader(0x2,3,2); // 0x200C2
	cmdbuf[1]=(u32)sockfd;
	cmdbuf[2]=input_opt;
	cmdbuf[3]=size;
	cmdbuf[4]=IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[5]=(u32)hostname;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__sslc_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret))context->sslchandle = cmdbuf[2];

	return ret;
}

static Result sslcipc_CreateCertChain(u32 type, u32 *contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x3 + type*0x5,0,0); // 0x30000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__sslc_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret))*contexthandle = cmdbuf[2];

	return ret;
}

static Result sslcipc_DestroyCertChain(u32 type, u32 contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x4 + type*0x5,1,0); // 0x40040
	cmdbuf[1]=contexthandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__sslc_servhandle)))return ret;

	return cmdbuf[1];
}

static Result sslcipc_CertChainAddCert(u32 type, u32 contexthandle, const u8 *cert, u32 certsize, u32 *cert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x5 + type*0x5,2,2); // 0x50082
	cmdbuf[1]=contexthandle;
	cmdbuf[2]=certsize;
	cmdbuf[3]=IPC_Desc_Buffer(certsize, IPC_BUFFER_R);
	cmdbuf[4]=(u32)cert;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__sslc_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret) && cert_contexthandle)*cert_contexthandle = cmdbuf[2];

	return ret;
}

static Result sslcipc_CertChainAddDefaultCert(u32 type, u32 contexthandle, u8 certID, u32 *cert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x6 + type*0x5,2,0); // 0x60080
	cmdbuf[1]=contexthandle;
	cmdbuf[2]=certID;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__sslc_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret) && cert_contexthandle)*cert_contexthandle = cmdbuf[2];

	return ret;
}

static Result sslcipc_CertChainRemoveCert(u32 type, u32 contexthandle, u32 cert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x7 + type*0x5,2,0); // 0x70080
	cmdbuf[1]=contexthandle;
	cmdbuf[2]=cert_contexthandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__sslc_servhandle)))return ret;

	return cmdbuf[1];
}

Result sslcOpenClientCertContext(const u8 *cert, u32 certsize, const u8 *key, u32 keysize, u32 *ClientCert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xD,2,4); // 0xD0084
	cmdbuf[1]=certsize;
	cmdbuf[2]=keysize;
	cmdbuf[3]=IPC_Desc_Buffer(certsize, IPC_BUFFER_R);
	cmdbuf[4]=(u32)cert;
	cmdbuf[5]=IPC_Desc_Buffer(keysize, IPC_BUFFER_R);
	cmdbuf[6]=(u32)key;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__sslc_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret))*ClientCert_contexthandle = cmdbuf[2];

	return ret;
}

Result sslcOpenDefaultClientCertContext(SSLC_DefaultClientCert certID, u32 *ClientCert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xE,1,0); // 0xE0040
	cmdbuf[1]=certID;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__sslc_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret))*ClientCert_contexthandle = cmdbuf[2];

	return ret;
}

Result sslcCloseClientCertContext(u32 ClientCert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xF,1,0); // 0xF0040
	cmdbuf[1]=ClientCert_contexthandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__sslc_servhandle)))return ret;

	return cmdbuf[1];
}

Result sslcSeedRNG(void)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x10,0,0); // 0x100000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__sslc_servhandle)))return ret;

	return cmdbuf[1];
}

Result sslcGenerateRandomData(u8 *buf, u32 size)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x11,1,2); // 0x110042
	cmdbuf[1]=size;
	cmdbuf[2]=IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[3]=(u32)buf;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__sslc_servhandle)))return ret;

	return cmdbuf[1];
}

static Result sslcipc_InitializeConnectionSession(sslcContext *context)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x12,1,2); // 0x120042
	cmdbuf[1]=context->sslchandle;
	cmdbuf[2]=IPC_Desc_CurProcessHandle();

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

static Result sslcipc_StartConnection(sslcContext *context)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x13,1,0); // 0x130040
	cmdbuf[1]=context->sslchandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

static Result sslcipc_StartConnectionGetOut(sslcContext *context, int *internal_retval, u32 *out)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x14,1,0); // 0x140040
	cmdbuf[1]=context->sslchandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret))
	{
		if(internal_retval)*internal_retval = cmdbuf[2];
		if(out)*out = cmdbuf[3];
	}

	return ret;
}

static Result sslcipc_DataTransfer(sslcContext *context, const void *buf, size_t len, u32 type)
{
	u32* cmdbuf=getThreadCommandBuffer();

	if(type >= 3)return -1;

	cmdbuf[0]=IPC_MakeHeader(0x15 + type,2,2); // 0x150082
	cmdbuf[1]=context->sslchandle;
	cmdbuf[2]=len;
	if(type<2)cmdbuf[3]=IPC_Desc_Buffer(len, IPC_BUFFER_W);
	if(type==2)cmdbuf[3]=IPC_Desc_Buffer(len, IPC_BUFFER_R);
	cmdbuf[4]=(u32)buf;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret))ret = cmdbuf[2];

	return ret;
}

static Result sslcipc_ContextSetValue(sslcContext *context, u32 type, u32 value)
{
	u32* cmdbuf=getThreadCommandBuffer();

	if(type >= 4)return -1;

	cmdbuf[0]=IPC_MakeHeader(0x18 + type,2,0); // 0x180080
	cmdbuf[1]=context->sslchandle;
	cmdbuf[2]=value;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result sslcContextGetProtocolCipher(sslcContext *context, char *outprotocols, u32 outprotocols_maxsize, char *outcipher, u32 outcipher_maxsize)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1C,3,4); // 0x1C00C4
	cmdbuf[1]=context->sslchandle;
	cmdbuf[2]=outprotocols_maxsize;
	cmdbuf[3]=outcipher_maxsize;
	cmdbuf[4]=IPC_Desc_Buffer(outprotocols_maxsize, IPC_BUFFER_W);
	cmdbuf[5]=(u32)outprotocols;
	cmdbuf[6]=IPC_Desc_Buffer(outcipher_maxsize, IPC_BUFFER_W);
	cmdbuf[7]=(u32)outcipher;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;
	ret = cmdbuf[1];

	return ret;
}

Result sslcContextGetState(sslcContext *context, u32 *out)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1D,1,0); // 0x1D0040
	cmdbuf[1]=context->sslchandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret))*out = cmdbuf[2];

	return ret;
}

static Result sslcipc_DestroyContext(sslcContext *context)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1E,1,0); // 0x1E0040
	cmdbuf[1]=context->sslchandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__sslc_servhandle)))return ret;

	return cmdbuf[1];
}

static Result sslcipc_ContextInitSharedmem(sslcContext *context, u32 size)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1F,2,2); // 0x1F0082
	cmdbuf[1]=context->sslchandle;
	cmdbuf[2]=size;
	cmdbuf[3]=IPC_Desc_SharedHandles(1);
	cmdbuf[4]=context->sharedmem_handle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result sslcAddCert(sslcContext *context, const u8 *buf, u32 size)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x20,2,2); // 0x200082
	cmdbuf[1]=context->sslchandle;
	cmdbuf[2]=size;
	cmdbuf[3]=IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[4]=(u32)buf;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;
	ret = cmdbuf[1];

	return ret;
}

Result sslcCreateRootCertChain(u32 *RootCertChain_contexthandle)
{
	return sslcipc_CreateCertChain(0, RootCertChain_contexthandle);
}

Result sslcDestroyRootCertChain(u32 RootCertChain_contexthandle)
{
	return sslcipc_DestroyCertChain(0, RootCertChain_contexthandle);
}

Result sslcAddTrustedRootCA(u32 RootCertChain_contexthandle, const u8 *cert, u32 certsize, u32 *cert_contexthandle)
{
	return sslcipc_CertChainAddCert(0, RootCertChain_contexthandle, cert, certsize, cert_contexthandle);
}

Result sslcRootCertChainAddDefaultCert(u32 RootCertChain_contexthandle, SSLC_DefaultRootCert certID, u32 *cert_contexthandle)
{
	return sslcipc_CertChainAddDefaultCert(0, RootCertChain_contexthandle, certID, cert_contexthandle);
}

Result sslcRootCertChainRemoveCert(u32 RootCertChain_contexthandle, u32 cert_contexthandle)
{
	return sslcipc_CertChainRemoveCert(0, RootCertChain_contexthandle, cert_contexthandle);
}

Result sslcCreate8CertChain(u32 *PinnedCertChain_contexthandle)
{
	return sslcipc_CreateCertChain(1, PinnedCertChain_contexthandle);
}

Result sslcDestroy8CertChain(u32 PinnedCertChain_contexthandle)
{
	return sslcipc_DestroyCertChain(1, PinnedCertChain_contexthandle);
}

Result sslc8CertChainAddCert(u32 PinnedCertChain_contexthandle, const u8 *cert, u32 certsize, u32 *cert_contexthandle)
{
	return sslcipc_CertChainAddCert(1, PinnedCertChain_contexthandle, cert, certsize, cert_contexthandle);
}

Result sslc8CertChainAddDefaultCert(u32 PinnedCertChain_contexthandle, u8 certID, u32 *cert_contexthandle)
{
	return sslcipc_CertChainAddDefaultCert(1, PinnedCertChain_contexthandle, certID, cert_contexthandle);
}

Result sslc8CertChainRemoveCert(u32 PinnedCertChain_contexthandle, u32 cert_contexthandle)
{
	return sslcipc_CertChainRemoveCert(1, PinnedCertChain_contexthandle, cert_contexthandle);
}

Result sslcCreateContext(sslcContext *context, int sockfd, u32 input_opt, const char *hostname)
{
	Result ret=0;

	memset(context, 0, sizeof(sslcContext));

	ret = SOCU_AddGlobalSocket(sockfd);
	if(R_FAILED(ret))return ret;

	sockfd = soc_get_fd(sockfd);
	if(sockfd < 0) {
		errno = -sockfd;
		return -1;
	}

	ret = sslcipc_CreateContext(context, sockfd, input_opt, hostname);
	if(R_FAILED(ret))return ret;

	ret = srvGetServiceHandle(&context->servhandle, "ssl:C");
	if(R_FAILED(ret)) {
		sslcipc_DestroyContext(context);
		return ret;
        }

	ret = sslcipc_InitializeConnectionSession(context);
	if(R_FAILED(ret)) {
		svcCloseHandle(context->servhandle);
		sslcipc_DestroyContext(context);
        }

	return ret;
}

Result sslcDestroyContext(sslcContext *context)
{
	Result ret=0;

	svcCloseHandle(context->servhandle);
	ret = sslcipc_DestroyContext(context);

	if(context->sharedmem_handle)svcCloseHandle(context->sharedmem_handle);

	memset(context, 0, sizeof(sslcContext));

	return ret;
}

Result sslcStartConnection(sslcContext *context, int *internal_retval, u32 *out)
{
	if(internal_retval || out)return sslcipc_StartConnectionGetOut(context, internal_retval, out);
	return sslcipc_StartConnection(context);
}

Result sslcRead(sslcContext *context, void *buf, size_t len, bool peek)
{
	u32 type = 0;

	if(peek)type = 1;

	return sslcipc_DataTransfer(context, buf, len, type);
}

Result sslcWrite(sslcContext *context, const void *buf, size_t len)
{
	return sslcipc_DataTransfer(context, buf, len, 2);
}

Result sslcContextSetRootCertChain(sslcContext *context, u32 handle)
{
	return sslcipc_ContextSetValue(context, 0, handle);
}

Result sslcContextSetClientCert(sslcContext *context, u32 handle)
{
	return sslcipc_ContextSetValue(context, 1, handle);
}

Result sslcContextSetHandle8(sslcContext *context, u32 handle)
{
	return sslcipc_ContextSetValue(context, 2, handle);
}

Result sslcContextClearOpt(sslcContext *context, u32 bitmask)
{
	return sslcipc_ContextSetValue(context, 3, bitmask);
}

Result sslcContextInitSharedmem(sslcContext *context, u8 *buf, u32 size)
{
	Result ret=0;

	ret = svcCreateMemoryBlock(&context->sharedmem_handle, (u32)buf, size, 1, 3);
	if(R_FAILED(ret))return ret;

	ret = sslcipc_ContextInitSharedmem(context, size);

	if(R_FAILED(ret))
	{
		svcCloseHandle(context->sharedmem_handle);
		context->sharedmem_handle = 0;
	}

	return ret;
}

