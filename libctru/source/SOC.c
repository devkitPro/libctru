#include <unistd.h>
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
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>

Handle SOCU_handle = 0;
static int SOCU_errno = 0;

#define NET_UNKNOWN_ERROR_OFFSET	-10000//This is from libogc network_wii.c.

static u8 _net_error_code_map[] = { //This is based on the array from libogc network_wii.c.
	0, // 0
 	E2BIG,
 	EACCES,
 	EADDRINUSE,
 	EADDRNOTAVAIL,
 	EAFNOSUPPORT, // 5
	EAGAIN,
	EALREADY,
	EBADF,
 	EBADMSG,
 	EBUSY, // 10
 	ECANCELED,
 	ECHILD,
 	ECONNABORTED,
 	ECONNREFUSED,
 	ECONNRESET, // 15
 	EDEADLK,
 	EDESTADDRREQ,
 	EDOM,
 	EDQUOT,
 	EEXIST, // 20
 	EFAULT,
 	EFBIG,
 	EHOSTUNREACH,
 	EIDRM,
 	EILSEQ, // 25
	EINPROGRESS,
 	EINTR,
 	EINVAL,
 	EIO,
	EISCONN, // 30
 	EISDIR,
 	ELOOP,
 	EMFILE,
 	EMLINK,
 	EMSGSIZE, // 35
 	EMULTIHOP,
 	ENAMETOOLONG,
 	ENETDOWN,
 	ENETRESET,
 	ENETUNREACH, // 40
 	ENFILE,
 	ENOBUFS,
 	ENODATA,
 	ENODEV,
 	ENOENT, // 45
 	ENOEXEC,
 	ENOLCK,
 	ENOLINK,
 	ENOMEM,
 	ENOMSG, // 50
 	ENOPROTOOPT,
 	ENOSPC,
 	ENOSR,
 	ENOSTR,
 	ENOSYS, // 55
 	ENOTCONN,
 	ENOTDIR,
 	ENOTEMPTY,
 	ENOTSOCK,
 	ENOTSUP, // 60
 	ENOTTY,
 	ENXIO,
 	EOPNOTSUPP,
 	EOVERFLOW,
 	EPERM, // 65
 	EPIPE,
 	EPROTO,
 	EPROTONOSUPPORT,
 	EPROTOTYPE,
 	ERANGE, // 70
 	EROFS,
 	ESPIPE,
 	ESRCH,
 	ESTALE,
 	ETIME, // 75
 	ETIMEDOUT,
};

static s32 _net_convert_error(s32 sock_retval)//This is based on the function from libogc network_wii.c.
{
	if (sock_retval >= 0) return sock_retval;
	if (sock_retval < -sizeof(_net_error_code_map)
		|| !_net_error_code_map[-sock_retval])
			return NET_UNKNOWN_ERROR_OFFSET + sock_retval;
	return -_net_error_code_map[-sock_retval];
}

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
	return _net_convert_error(cmdbuf[2]);
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
	if(ret==0)ret =_net_convert_error(cmdbuf[2]);
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
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
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
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	SOCU_errno = ret;

	if(ret!=0)return -1;
	return 0;
}

int accept(int sockfd, struct sockaddr *addr, int *addrlen)
{
	int ret=0;
	int tmp_addrlen=0x1c;
	u32 *cmdbuf = getThreadCommandBuffer();
	u8 tmpaddr[0x1c];
	u32 saved_threadstorage[2];

	memset(tmpaddr, 0, 0x1c);

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
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret>=0 && addr!=NULL)
	{
		addr->sa_family = tmpaddr[1];
		if(*addrlen > tmpaddr[0])*addrlen = tmpaddr[0];
		memcpy(addr->sa_data, &tmpaddr[2], *addrlen - 2);
	}

	if(ret<0)return -1;
	return ret;
}

int bind(int sockfd, const struct sockaddr *addr, int addrlen)
{
	int ret=0;
	int tmp_addrlen=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u8 tmpaddr[0x1c];

	memset(tmpaddr, 0, 0x1c);

	if(addr->sa_family == AF_INET)
	{
		tmp_addrlen = 8;
	}
	else
	{
		tmp_addrlen = 0x1c;
	}

	if(addrlen < tmp_addrlen)
	{
		SOCU_errno = -EINVAL;
		return -1;
	}

	tmpaddr[0] = tmp_addrlen;
	tmpaddr[1] = addr->sa_family;
	memcpy(&tmpaddr[2], &addr->sa_data, tmp_addrlen-2);

	cmdbuf[0] = 0x00050084;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)tmp_addrlen;
	cmdbuf[3] = 0x20;
	cmdbuf[5] = (((u32)tmp_addrlen)<<14) | 2;
	cmdbuf[6] = (u32)tmpaddr;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	SOCU_errno = ret;

	if(ret<0)return -1;
	return 0;
}

int connect(int sockfd, const struct sockaddr *addr, int addrlen)
{
	int ret=0;
	int tmp_addrlen=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u8 tmpaddr[0x1c];

	memset(tmpaddr, 0, 0x1c);

	if(addr->sa_family == AF_INET)
	{
		tmp_addrlen = 8;
	}
	else
	{
		tmp_addrlen = 0x1c;
	}

	if(addrlen < tmp_addrlen)
	{
		SOCU_errno = -EINVAL;
		return -1;
	}

	tmpaddr[0] = tmp_addrlen;
	tmpaddr[1] = addr->sa_family;
	memcpy(&tmpaddr[2], &addr->sa_data, tmp_addrlen-2);

	cmdbuf[0] = 0x00060084;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)addrlen;
	cmdbuf[3] = 0x20;
	cmdbuf[5] = (((u32)tmp_addrlen)<<14) | 2;
	cmdbuf[6] = (u32)tmpaddr;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	SOCU_errno = ret;

	if(ret<0)return -1;
	return 0;
}

int socuipc_cmd7(int sockfd, void *buf, int len, int flags, struct sockaddr *src_addr, int *addrlen)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 tmp_addrlen=0;
	u8 tmpaddr[0x1c];
	u32 saved_threadstorage[2];

	memset(tmpaddr, 0, 0x1c);

	if(src_addr)tmp_addrlen = 0x1c;

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
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret>0 && src_addr!=NULL)
	{
		src_addr->sa_family = tmpaddr[1];
		if(*addrlen > tmpaddr[0])*addrlen = tmpaddr[0];
		memcpy(src_addr->sa_data, &tmpaddr[2], *addrlen - 2);
	}

	if(ret<0)return -1;
	return ret;
}

int socuipc_cmd8(int sockfd, void *buf, int len, int flags, struct sockaddr *src_addr, int *addrlen)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();	
	u32 tmp_addrlen=0;
	u8 tmpaddr[0x1c];
	u32 saved_threadstorage[4];

	if(src_addr)tmp_addrlen = 0x1c;

	memset(tmpaddr, 0, 0x1c);

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
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret>0 && src_addr!=NULL)
	{
		src_addr->sa_family = tmpaddr[1];
		if(*addrlen > tmpaddr[0])*addrlen = tmpaddr[0];
		memcpy(src_addr->sa_data, &tmpaddr[2], *addrlen - 2);
	}

	if(ret<0)return -1;
	return ret;
}

int socuipc_cmd9(int sockfd, const void *buf, int len, int flags, const struct sockaddr *dest_addr, int addrlen)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 tmp_addrlen=0;
	u8 tmpaddr[0x1c];

	memset(tmpaddr, 0, 0x1c);

	if(dest_addr)
	{
		if(dest_addr->sa_family == AF_INET)
		{
			tmp_addrlen = 8;
		}
		else
		{
			tmp_addrlen = 0x1c;
		}

		if(addrlen < tmp_addrlen)
		{
			SOCU_errno = -EINVAL;
			return -1;
		}

		tmpaddr[0] = tmp_addrlen;
		tmpaddr[1] = dest_addr->sa_family;
		memcpy(&tmpaddr[2], &dest_addr->sa_data, tmp_addrlen-2);
	}

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
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret<0)return -1;
	return ret;
}

int socuipc_cmda(int sockfd, const void *buf, int len, int flags, const struct sockaddr *dest_addr, int addrlen)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 tmp_addrlen=0;
	u8 tmpaddr[0x1c];

	memset(tmpaddr, 0, 0x1c);

	if(dest_addr)
	{
		if(dest_addr->sa_family == AF_INET)
		{
			tmp_addrlen = 8;
		}
		else
		{
			tmp_addrlen = 0x1c;
		}

		if(addrlen < tmp_addrlen)
		{
			SOCU_errno = -EINVAL;
			return -1;
		}

		tmpaddr[0] = tmp_addrlen;
		tmpaddr[1] = dest_addr->sa_family;
		memcpy(&tmpaddr[2], &dest_addr->sa_data, tmp_addrlen-2);
	}

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
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret<0)return -1;
	return ret;
}

int recvfrom(int sockfd, void *buf, int len, int flags, struct sockaddr *src_addr, int *addrlen)
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

int getsockopt(int sockfd, int level, int option_name, void * data, int * data_len)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 saved_threadstorage[2];

	cmdbuf[0] = 0x00110102;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)level;
	cmdbuf[3] = (u32)option_name;
	cmdbuf[4] = (u32)*data_len;
	cmdbuf[5] = 0x20;

	saved_threadstorage[0] = cmdbuf[0x100>>2];
	saved_threadstorage[1] = cmdbuf[0x104>>2];

	cmdbuf[0x100>>2] = ((*data_len)<<14) | 2;
	cmdbuf[0x104>>2] = (u32)data;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	cmdbuf[0x100>>2] = saved_threadstorage[0];
	cmdbuf[0x104>>2] = saved_threadstorage[1];

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret==0)*data_len = cmdbuf[3];

	if(ret<0)return -1;
	return ret;
}

int setsockopt(int sockfd, int level, int option_name, const void * data, int data_len)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00120104;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)level;
	cmdbuf[3] = (u32)option_name;
	cmdbuf[4] = (u32)data_len;
	cmdbuf[5] = 0x20;
	cmdbuf[7] = (data_len<<14) | 0x2402;
	cmdbuf[8] = (u32)data;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret<0)return -1;
	return ret;
}

int fcntl(int sockfd, int cmd, ...)
{
	int ret=0;
	int arg=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	va_list args;
    	va_start(args, cmd);

	if(cmd!=F_GETFL && cmd!=F_SETFL)
	{
		SOCU_errno = -EINVAL;
		return -1;
	}

	if(cmd==F_SETFL)
	{
		arg = va_arg(args, int);

		if(arg && arg!=O_NONBLOCK)
		{
			SOCU_errno = -EINVAL;
			return -1;
		}

		if(arg==O_NONBLOCK)arg = 0x4;
	}

	cmdbuf[0] = 0x001300C2;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)cmd;
	cmdbuf[3] = (u32)arg;
	cmdbuf[4] = 0x20;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret<0)return -1;
	return ret;
}

int sockatmark(int sockfd)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00150042;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = 0x20;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret<0)return -1;
	return ret;
}

long gethostid()
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00160000;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = cmdbuf[2];

	return ret;
}

int getsockname(int sockfd, struct sockaddr *addr, int * addr_len)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 saved_threadstorage[2];
	u8 tmpaddr[0x1c];

	cmdbuf[0] = 0x00170082;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = 0x1c;
	cmdbuf[3] = 0x20;

	saved_threadstorage[0] = cmdbuf[0x100>>2];
	saved_threadstorage[1] = cmdbuf[0x104>>2];

	cmdbuf[0x100>>2] = (0x1c<<14) | 2;
	cmdbuf[0x104>>2] = (u32)tmpaddr;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	cmdbuf[0x100>>2] = saved_threadstorage[0];
	cmdbuf[0x104>>2] = saved_threadstorage[1];

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret==0)
	{
		addr->sa_family = tmpaddr[1];
		if(*addr_len > tmpaddr[0])*addr_len = tmpaddr[0];
		memset(addr, 0, sizeof(struct sockaddr));
		memcpy(addr->sa_data, &tmpaddr[2], *addr_len - 2);
	}

	if(ret<0)return -1;
	return ret;
}

int getpeername(int sockfd, struct sockaddr *addr, int * addr_len)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 saved_threadstorage[2];
	u8 tmpaddr[0x1c];

	cmdbuf[0] = 0x00180082;
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = 0x1c;
	cmdbuf[3] = 0x20;

	saved_threadstorage[0] = cmdbuf[0x100>>2];
	saved_threadstorage[1] = cmdbuf[0x104>>2];

	cmdbuf[0x100>>2] = (0x1c<<14) | 2;
	cmdbuf[0x104>>2] = (u32)tmpaddr;

	if((ret = svc_sendSyncRequest(SOCU_handle))!=0)return ret;

	cmdbuf[0x100>>2] = saved_threadstorage[0];
	cmdbuf[0x104>>2] = saved_threadstorage[1];

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
	if(ret<0)SOCU_errno = ret;

	if(ret==0)
	{
		addr->sa_family = tmpaddr[1];
		if(*addr_len > tmpaddr[0])*addr_len = tmpaddr[0];
		memset(addr, 0, sizeof(struct sockaddr));
		memcpy(addr->sa_data, &tmpaddr[2], *addr_len - 2);
	}

	if(ret<0)return -1;
	return ret;
}

