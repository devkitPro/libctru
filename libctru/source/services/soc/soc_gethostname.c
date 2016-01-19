#include <arpa/inet.h>
#include <3ds/result.h>       // To be removed when dkA patch to newlib is applied
#include <3ds/services/soc.h> // To be changed to unistd.h when dkA patch to newlib is applied


// The 3DS doesn't give any host name for its own IP through gethostbyaddr
// For compatibility, the host ASCII name will be given (IPv4 dotted notation)
int gethostname(char *name, size_t namelen)
{
	long hostid = gethostid();
	const char * hostname = inet_ntop(AF_INET,&hostid,name,namelen);
	if(hostname == NULL)return -1;
	return 0;
}
