/*
  srv.c _ Service manager.
*/

#include <string.h>
#include <3ds/types.h>
#include <3ds/srv.h>
#include <3ds/svc.h>


/*
  The homebrew loader can choose to supply a list of service handles that have
  been "stolen" from other processes that have been compromised. This allows us
  to access services that are normally restricted from the current process.

  For every service requested by the application, we shall first check if the
  list given to us contains the requested service and if so use it. If we don't
  find the service in that list, we ask the service manager and hope for the
  best.
 */

typedef struct {
	u32 num;

	struct {
		char name[8];
		Handle handle;
	} services[];
} service_list_t;

extern service_list_t* __service_ptr;

static Handle g_srv_handle = 0;


static int __name_cmp(const char* a, const char* b) {
	u32 i;

	for(i=0; i<8; i++) {
		if(a[i] != b[i])
			return 1;
		if(a[i] == '\0')
			return 0;
	}

	return 0;
}

Handle __get_handle_from_list(const char* name) {
	if((u32)__service_ptr == 0)
		return 0;

	u32 i, num = __service_ptr->num;

	for(i=0; i<num; i++) {
		if(__name_cmp(__service_ptr->services[i].name, name) == 0)
			return __service_ptr->services[i].handle;
	}

	return 0;
}

void __destroy_handle_list(void) {
	if((u32)__service_ptr == 0)
		return;

	u32 i, num = __service_ptr->num;

	for(i=0; i<num; i++)
		svcCloseHandle(__service_ptr->services[i].handle);

	__service_ptr->num = 0;
}


Result srvInit()
{
	Result rc = 0;

	if(g_srv_handle != 0) return rc;

	if((rc = svcConnectToPort(&g_srv_handle, "srv:")))return rc;

	if((rc = srvRegisterClient())) {
		svcCloseHandle(g_srv_handle);
		g_srv_handle = 0;
	}

	return rc;
}

Result srvExit()
{
	if(g_srv_handle != 0)svcCloseHandle(g_srv_handle);

	g_srv_handle = 0;
	return 0;
}

Handle *srvGetSessionHandle()
{
	return &g_srv_handle;
}

Result srvRegisterClient()
{
	Result rc = 0;
	
	u32* cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = 0x10002;
	cmdbuf[1] = 0x20;

	if((rc = svcSendSyncRequest(g_srv_handle)))return rc;

	return cmdbuf[1];
}

Result srvGetServiceHandle(Handle* out, const char* name)
{
	Result rc = 0;

	/* Look in service-list given to us by loader. If we find find a match,
	   we return it. */
	Handle h = __get_handle_from_list(name);

	if(h != 0) {
		return svcDuplicateHandle(out, h);
	}

	/* Normal request to service manager. */
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x50100;
	strcpy((char*) &cmdbuf[1], name);
	cmdbuf[3] = strlen(name);
	cmdbuf[4] = 0x0;
	
	if((rc = svcSendSyncRequest(g_srv_handle)))return rc;

	*out = cmdbuf[3];
	return cmdbuf[1];
}

Result srvRegisterService(Handle* out, const char* name, int maxSessions)
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x30100;
	strcpy((char*) &cmdbuf[1], name);
	cmdbuf[3] = strlen(name);
	cmdbuf[4] = maxSessions;
	
	Result rc;
	if((rc = svcSendSyncRequest(g_srv_handle)))return rc;

	*out = cmdbuf[3];
	return cmdbuf[1];
}

Result srvUnregisterService(const char* name)
{
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x400C0;
	strcpy((char*) &cmdbuf[1], name);
	cmdbuf[3] = strlen(name);
	
	Result rc;
	if((rc = svcSendSyncRequest(g_srv_handle)))return rc;

	return cmdbuf[1];
}

// Old srv:pm interface, will only work on systems where srv:pm was a port (<7.X)
Result srvPmInit()
{	
	Result rc = 0;
	
	if((rc = svcConnectToPort(&g_srv_handle, "srv:pm")))return rc;
	
	if((rc = srvRegisterClient())) {
		svcCloseHandle(g_srv_handle);
		g_srv_handle = 0;
	}

	return rc;
}

Result srvRegisterProcess(u32 procid, u32 count, void *serviceaccesscontrol)
{
	Result rc = 0;
	
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x04030082; // <7.x
	cmdbuf[1] = procid;
	cmdbuf[2] = count;
	cmdbuf[3] = (count << 16) | 2;
	cmdbuf[4] = (u32)serviceaccesscontrol;
	
	if((rc = svcSendSyncRequest(g_srv_handle))) return rc;
		
	return cmdbuf[1];
}

Result srvUnregisterProcess(u32 procid)
{
	Result rc = 0;
	
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x04040040; // <7.x
	cmdbuf[1] = procid;
	
	if((rc = svcSendSyncRequest(g_srv_handle))) return rc;
		
	return cmdbuf[1];
}
