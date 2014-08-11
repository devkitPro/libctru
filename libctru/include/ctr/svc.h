#ifndef SVC_H
#define SVC_H

typedef enum{
	MEMOP_FREE = 1,
	MEMOP_RESERVE = 2,
	MEMOP_COMMIT = 3,
	MEMOP_MAP = 4,
	MEMOP_UNMAP = 5,
	MEMOP_PROTECT = 6,
	MEMOP_REGION_APP = 0x100,
	MEMOP_REGION_SYSTEM = 0x200,
	MEMOP_REGION_BASE = 0x300,
	MEMOP_LINEAR = 0x1000,
}MEMORY_OPERATION;

	u32* getThreadCommandBuffer(void);
	Result svc_controlMemory(u32* outaddr, u32 addr0, u32 addr1, u32 size, u32 operation, u32 permissions); //(outaddr is usually the same as the input addr0)
	void svc_exitProcess(void);
	Result svc_createThread(Handle* thread, ThreadFunc entrypoint, u32 arg, u32* stacktop, s32 threadpriority, s32 processorid);
	void svc_exitThread();
	void svc_sleepThread(s64 ns);
	Result svc_createMutex(Handle* mutex, bool initialLocked);
	Result svc_releaseMutex(Handle handle);
	Result svc_releaseSemaphore(s32* count, Handle semaphore, s32 releaseCount);
	Result svc_createEvent(Handle* event, u8 resettype);
	Result svc_signalEvent(Handle handle);
	Result svc_clearEvent(Handle handle);
	Result svc_createMemoryBlock(Handle* memblock, u32 addr, u32 size, u32 mypermission, u32 otherpermission);
	Result svc_mapMemoryBlock(Handle memblock, u32 addr, u32 mypermissions, u32 otherpermission);
	Result svc_unmapMemoryBlock(Handle memblock, u32 addr);
	Result svc_waitSynchronization1(Handle handle, s64 nanoseconds);
	Result svc_waitSynchronizationN(s32* out, Handle* handles, s32 handlecount, bool waitAll, s64 nanoseconds);
	Result svc_arbitrateAddress(Handle arbiter, u32 addr, u8 type, s32 value, s64 nanoseconds);
	Result svc_closeHandle(Handle handle);
	u64 svc_getSystemTick();
	Result svc_getSystemInfo(s64* out, u32 type, s32 param);
	Result svc_connectToPort(volatile Handle* out, const char* portName);
	Result svc_sendSyncRequest(Handle session);
	Result svc_getProcessId(u32 *out, Handle handle);

#endif
