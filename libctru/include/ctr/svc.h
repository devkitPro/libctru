#ifndef SVC_H
#define SVC_H

	u32* getThreadCommandBuffer(void);
	
	void svc_exitProcess(void);
	void svc_sleepThread(s64 ns);
	Result svc_releaseMutex(Handle handle);
	Result svc_controlMemory(u32* outaddr, u32 addr0, u32 addr1, u32 size, u32 operation, u32 permissions); //(outaddr is usually the same as the input addr0)
	Result svc_createEvent(Handle* event, u8 resettype);
	Result svc_clearEvent(Handle handle);
	Result svc_mapMemoryBlock(Handle memblock, u32 addr, u32 mypermissions, u32 otherpermission);
	Result svc_waitSynchronization1(Handle handle, s64 nanoseconds);
	Result svc_waitSynchronizationN(s32* out, Handle* handles, s32 handlecount, bool waitAll, s64 nanoseconds);
	Result svc_closeHandle(Handle handle);
	Result svc_connectToPort(volatile Handle* out, const char* portName);
	Result svc_sendSyncRequest(Handle session);

#endif
