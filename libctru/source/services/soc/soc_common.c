#include "soc_common.h"
#include <errno.h>
#include <sys/iosupport.h>

Handle	SOCU_handle = 0;
Handle	socMemhandle = 0;
int h_errno = 0;

//This is based on the array from libogc network_wii.c.
static u8 _net_error_code_map[] = {
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

#define NET_UNKNOWN_ERROR_OFFSET	-10000//This is from libogc network_wii.c.

//This is based on the function from libogc network_wii.c.
s32 _net_convert_error(s32 sock_retval)
{
	if(sock_retval >= 0)
		return sock_retval;

	if(sock_retval < -sizeof(_net_error_code_map)
	|| !_net_error_code_map[-sock_retval])
		return NET_UNKNOWN_ERROR_OFFSET + sock_retval;
	return -_net_error_code_map[-sock_retval];
}
