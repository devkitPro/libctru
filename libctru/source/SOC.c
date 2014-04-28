#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/svc.h>
#include <ctr/srv.h>
#include <ctr/SOC.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

Handle SOCU_handle = 0;
static int SOCU_errno = 0;

Result socu_cmd1(Handle memhandle, u32 memsize)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00010044;
	cmdbuf[1] = memsize;
	cmdbuf[2] = 0x20;
	cmdbuf[4] = 0;
	cmdbuf[5] = memhandle;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	return cmdbuf[1];
}

Result SOC_Shutdown()
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00190000;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	svc_closeHandle(SOCU_handle);

	return cmdbuf[1];
}

Result SOC_Initialize(u32 *context_addr, u32 context_size)
{
	Result ret=0;
	Handle memhandle = 0;

	ret = svc_createMemoryBlock(&memhandle, (u32)context_addr, context_size, 0, 3);
	if(ret!=0)return ret;

	if((ret = srv_getServiceHandle(NULL, &SOCU_handle, "soc:U"))!=0)return ret;

	return socu_cmd1(memhandle, context_size);
}

int SOC_GetErrno()
{
	return SOCU_errno;
}

int socket(int domain, int type, int protocol)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000200C2;
	cmdbuf[1] = domain;
	cmdbuf[2] = type;
	cmdbuf[3] = protocol;
	cmdbuf[4] = 0x20;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	SOCU_errno = ret;

	if(ret!=0)return -1;
	return (int)cmdbuf[2];
}

int closesocket(int sockfd)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000B0042;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = 0x20;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = (int)cmdbuf[2];
	SOCU_errno = ret;

	if(ret!=0)return -1;
	return 0;
}

int shutdown(int sockfd, int shutdown_type)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000C0082;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)shutdown_type;
	cmdbuf[3] = 0x20;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = (int)cmdbuf[2];
	SOCU_errno = ret;

	if(ret!=0)return -1;
	return 0;
}

int listen(int sockfd, int max_connections)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00030082;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)max_connections;
	cmdbuf[3] = 0x20;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = (int)cmdbuf[2];
	SOCU_errno = ret;

	if(ret!=0)return -1;
	return 0;
}

int accept(int sockfd, struct sockaddr *addr, int *addrlen)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u8 tmpaddr[8];
	int tmp_addrlen=8;
	u32 saved_threadstorage[2];

	memset(tmpaddr, 0, 8);

	cmdbuf[0] = 0x00040082;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)tmp_addrlen;
	cmdbuf[3] = 0x20;

	saved_threadstorage[0] = cmdbuf[0x100>>2];
	saved_threadstorage[1] = cmdbuf[0x104>>2];

	cmdbuf[0x100>>2] = (tmp_addrlen<<14) | 2;
	cmdbuf[0x104>>2] = (u32)tmpaddr;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	cmdbuf[0x100>>2] = saved_threadstorage[0];
	cmdbuf[0x104>>2] = saved_threadstorage[1];

	ret = (int)cmdbuf[1];
	if(ret==0)ret = (int)cmdbuf[2];
	if(ret<0)SOCU_errno = ret;

	if(ret>=0 && addr!=NULL)
	{
		*addrlen = tmpaddr[0];
		memset(addr, 0, sizeof(struct sockaddr));
		addr->sa_family = tmpaddr[1];
		memcpy(&addr->sa_data, &tmpaddr[2], tmp_addrlen-2);
	}

	if(ret<0)return -1;
	return ret;
}

int bind(int sockfd, const struct sockaddr *addr, int addrlen)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	//struct sockaddr_in *inaddr = (struct sockaddr_in*)addr;
	u8 tmpaddr[8];

	addrlen = 8;
	tmpaddr[0] = 8;
	tmpaddr[1] = addr->sa_family;
	memcpy(&tmpaddr[2], &addr->sa_data, addrlen-2);

	cmdbuf[0] = 0x00050084;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)addrlen;
	cmdbuf[3] = 0x20;
	cmdbuf[5] = (((u32)addrlen)<<14) | 2;
	cmdbuf[6] = (u32)tmpaddr;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = (int)cmdbuf[2];
	SOCU_errno = ret;

	if(ret<0)return -1;
	return 0;
}

int connect(int sockfd, const struct sockaddr *addr, int addrlen)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	//struct sockaddr_in *inaddr = (struct sockaddr_in*)addr;
	u8 tmpaddr[8];

	addrlen = 8;
	tmpaddr[0] = 8;
	tmpaddr[1] = addr->sa_family;
	memcpy(&tmpaddr[2], &addr->sa_data, addrlen-2);

	cmdbuf[0] = 0x00060084;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)addrlen;
	cmdbuf[3] = 0x20;
	cmdbuf[5] = (((u32)addrlen)<<14) | 2;
	cmdbuf[6] = (u32)tmpaddr;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = (int)cmdbuf[2];
	SOCU_errno = ret;

	if(ret<0)return -1;
	return 0;
}

int socuipc_cmd7(int sockfd, void *buf, int len, int flags, struct sockaddr *src_addr, int *addrlen)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u8 tmpaddr[8];
	u32 tmp_addrlen=0;
	u32 saved_threadstorage[2];

	memset(tmpaddr, 0, 8);

	cmdbuf[0] = 0x00070104;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)len;
	cmdbuf[3] = (u32)flags;
	cmdbuf[4] = (u32)tmp_addrlen;
	cmdbuf[5] = 0x20;
	cmdbuf[7] = (((u32)len)<<4) | 12;
	cmdbuf[8] = (u32)buf;

	saved_threadstorage[0] = cmdbuf[0x100>>2];
	saved_threadstorage[1] = cmdbuf[0x104>>2];

	cmdbuf[0x100>>2] = (tmp_addrlen<<14) | 2;
	cmdbuf[0x104>>2] = (u32)tmpaddr;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	cmdbuf[0x100>>2] = saved_threadstorage[0];
	cmdbuf[0x104>>2] = saved_threadstorage[1];

	ret = (int)cmdbuf[1];
	if(ret==0)ret = (int)cmdbuf[2];
	if(ret<0)SOCU_errno = ret;

	if(ret<0)return -1;
	return ret;
}

int socuipc_cmd8(int sockfd, void *buf, int len, int flags, struct sockaddr *src_addr, int *addrlen)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u8 tmpaddr[8];
	u32 tmp_addrlen=0;
	u32 saved_threadstorage[4];

	memset(tmpaddr, 0, 8);

	cmdbuf[0] = 0x00080102;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)len;
	cmdbuf[3] = (u32)flags;
	cmdbuf[4] = (u32)tmp_addrlen;
	cmdbuf[5] = 0x20;

	saved_threadstorage[0] = cmdbuf[0x100>>2];
	saved_threadstorage[1] = cmdbuf[0x104>>2];
	saved_threadstorage[2] = cmdbuf[0x108>>2];
	saved_threadstorage[3] = cmdbuf[0x10c>>2];
	
	cmdbuf[0x100>>2] = (((u32)len)<<14) | 2;
	cmdbuf[0x104>>2] = (u32)buf;
	cmdbuf[0x108>>2] = (tmp_addrlen<<14) | 2;
	cmdbuf[0x10c>>2] = (u32)tmpaddr;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	cmdbuf[0x100>>2] = saved_threadstorage[0];
	cmdbuf[0x104>>2] = saved_threadstorage[1];
	cmdbuf[0x108>>2] = saved_threadstorage[2];
	cmdbuf[0x10c>>2] = saved_threadstorage[3];

	ret = (int)cmdbuf[1];
	if(ret==0)ret = (int)cmdbuf[2];
	if(ret<0)SOCU_errno = ret;

	if(ret<0)return -1;
	return ret;
}

int socuipc_cmd9(int sockfd, const void *buf, int len, int flags, const struct sockaddr *dest_addr, int addrlen)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u8 tmpaddr[8];
	u32 tmp_addrlen=0;

	memset(tmpaddr, 0, 8);

	cmdbuf[0] = 0x00090106;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)len;
	cmdbuf[3] = (u32)flags;
	cmdbuf[4] = (u32)tmp_addrlen;
	cmdbuf[5] = 0x20;
	cmdbuf[7] = (tmp_addrlen<<14) | 0x402;
	cmdbuf[8] = (u32)tmpaddr;
	cmdbuf[9] = (((u32)len)<<4) | 10;
	cmdbuf[10] = (u32)buf;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = (int)cmdbuf[2];
	if(ret<0)SOCU_errno = ret;

	if(ret<0)return -1;
	return ret;
}

int socuipc_cmda(int sockfd, const void *buf, int len, int flags, const struct sockaddr *dest_addr, int addrlen)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u8 tmpaddr[8];
	int tmp_addrlen=0;

	memset(tmpaddr, 0, 8);

	cmdbuf[0] = 0x000A0106;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)len;
	cmdbuf[3] = (u32)flags;
	cmdbuf[4] = (u32)tmp_addrlen;
	cmdbuf[5] = 0x20;
	cmdbuf[7] = (((u32)len)<<14) | 0x802;
	cmdbuf[8] = (u32)buf;
	cmdbuf[9] = (tmp_addrlen<<14) | 0x402;
	cmdbuf[10] = (u32)tmpaddr;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = (int)cmdbuf[2];
	if(ret<0)SOCU_errno = ret;

	if(ret<0)return -1;
	return ret;
}

int recvfrom(int sockfd, void *buf, int len, int flags, struct sockaddr *src_addr, int *addrlen)//UDP is not supported for these since the input/output sockaddr is not used.
{
	if(len<0x2000)return socuipc_cmd8(sockfd, buf, len, flags, src_addr, addrlen);
	return socuipc_cmd7(sockfd, buf, len, flags, src_addr, addrlen);
}

int sendto(int sockfd, const void *buf, int len, int flags, const struct sockaddr *dest_addr, int addrlen)
{
	if(len<0x2000)return socuipc_cmda(sockfd, buf, len, flags, dest_addr, addrlen);
	return socuipc_cmd9(sockfd, buf, len, flags, (struct sockaddr*)dest_addr, addrlen);
}

int recv(int sockfd, void *buf, int len, int flags)
{
	return recvfrom(sockfd, buf, len, flags, NULL, 0);
}

int send(int sockfd, const void *buf, int len, int flags)
{
	return sendto(sockfd, buf, len, flags, NULL, 0);
}

