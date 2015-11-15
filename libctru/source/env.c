#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/env.h>

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

extern void* __service_ptr;

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

Handle envGetHandle(const char* name) {
	if(__service_ptr == NULL)
		return 0;

	service_list_t* service_list = (service_list_t*) __service_ptr;
	u32 i, num = service_list->num;

	for(i=0; i<num; i++) {
		if(__name_cmp(service_list->services[i].name, name) == 0)
			return service_list->services[i].handle;
	}

	return 0;
}

void envDestroyHandles(void) {
	if(__service_ptr == NULL)
		return;

	service_list_t* service_list = (service_list_t*) __service_ptr;
	u32 i, num = service_list->num;

	for(i=0; i<num; i++)
		svcCloseHandle(service_list->services[i].handle);

	service_list->num = 0;
}

