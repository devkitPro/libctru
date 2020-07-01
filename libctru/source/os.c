#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/os.h>
#include <3ds/svc.h>
#include <3ds/services/ptmsysm.h>

#include <sys/time.h>
#include <reent.h>
#include <unistd.h>

// Work around the VFP not supporting 64-bit integer <--> floating point conversion
static inline double u64_to_double(u64 value) {
	return (((double)(u32)(value >> 32))*0x100000000ULL+(u32)value);
}

typedef struct {
	u64 date_time;
	u64 update_tick;
	//...
} datetime_t;

#define __datetime_selector        (*(vu32*)0x1FF81000)
#define __datetime0 (*(volatile datetime_t*)0x1FF81020)
#define __datetime1 (*(volatile datetime_t*)0x1FF81040)

__attribute__((weak)) bool __ctru_speedup = false;

//---------------------------------------------------------------------------------
u32 osConvertVirtToPhys(const void* addr) {
//---------------------------------------------------------------------------------
	u32 vaddr = (u32)addr;
#define CONVERT_REGION(_name) \
	if (vaddr >= OS_##_name##_VADDR && vaddr < (OS_##_name##_VADDR + OS_##_name##_SIZE)) \
		return vaddr + (OS_##_name##_PADDR - OS_##_name##_VADDR);

	CONVERT_REGION(FCRAM);
	CONVERT_REGION(VRAM);
	CONVERT_REGION(OLD_FCRAM);
	CONVERT_REGION(DSPRAM);
	CONVERT_REGION(QTMRAM);
	CONVERT_REGION(MMIO);

#undef CONVERT_REGION
	return 0;
}

//---------------------------------------------------------------------------------
void* osConvertOldLINEARMemToNew(const void* addr) {
//---------------------------------------------------------------------------------
	u32 vaddr = (u32)addr;
	if (vaddr >= OS_FCRAM_VADDR && vaddr < (OS_FCRAM_VADDR+OS_FCRAM_SIZE))
		return (void*)vaddr;
	if (vaddr >= OS_OLD_FCRAM_VADDR && vaddr < (OS_FCRAM_VADDR+OS_OLD_FCRAM_SIZE))
		return (void*)(vaddr + (OS_FCRAM_VADDR-OS_OLD_FCRAM_VADDR));
	return NULL;
}

//---------------------------------------------------------------------------------
static datetime_t getSysTime(void) {
//---------------------------------------------------------------------------------
	u32 s1, s2 = __datetime_selector & 1;
	datetime_t dt;

	do {
		s1 = s2;
		if(!s1)
			dt = __datetime0;
		else
			dt = __datetime1;
		s2 = __datetime_selector & 1;
	} while(s2 != s1);

	return dt;
}

//---------------------------------------------------------------------------------
int __libctru_gtod(struct _reent *ptr, struct timeval *tp, struct timezone *tz) {
//---------------------------------------------------------------------------------
	if (tp != NULL) {

		datetime_t dt = getSysTime();

		u64 delta = svcGetSystemTick() - dt.update_tick;

		u32 offset =  (u32)(u64_to_double(delta)/CPU_TICKS_PER_USEC);

		// adjust from 1900 to 1970
		u64 now = ((dt.date_time - 2208988800000ULL) * 1000) + offset;

		tp->tv_sec =  u64_to_double(now)/1000000.0;
		tp->tv_usec = now - ((tp->tv_sec) * 1000000);

	}

	if (tz != NULL) {
		tz->tz_minuteswest = 0;
		tz->tz_dsttime = 0;
	}

	return 0;

}

// Returns number of milliseconds since 1st Jan 1900 00:00.
//---------------------------------------------------------------------------------
u64 osGetTime(void) {
//---------------------------------------------------------------------------------
	datetime_t dt = getSysTime();

	u64 delta = svcGetSystemTick() - dt.update_tick;

	return dt.date_time + (u32)(u64_to_double(delta)/CPU_TICKS_PER_MSEC);
}

//---------------------------------------------------------------------------------
double osTickCounterRead(const TickCounter* cnt) {
//---------------------------------------------------------------------------------
	return u64_to_double(cnt->elapsed) / CPU_TICKS_PER_MSEC;
}

//---------------------------------------------------------------------------------
const char* osStrError(Result error) {
//---------------------------------------------------------------------------------
	switch(R_SUMMARY(error)) {
	case RS_SUCCESS:
		return "Success.";
	case RS_NOP:
		return "Nothing happened.";
	case RS_WOULDBLOCK:
		return "Would block.";
	case RS_OUTOFRESOURCE:
		return "Not enough resources.";
	case RS_NOTFOUND:
		return "Not found.";
	case RS_INVALIDSTATE:
		return "Invalid state.";
	case RS_NOTSUPPORTED:
		return "Unsupported.";
	case RS_INVALIDARG:
		return "Invalid argument.";
	case RS_WRONGARG:
		return "Wrong argument.";
	case RS_CANCELED:
		return "Cancelled.";
	case RS_STATUSCHANGED:
		return "Status changed.";
	case RS_INTERNAL:
		return "Internal error.";
	default:
		return "Unknown.";
	}
}

void __ctru_speedup_config(void)
{
	if (R_SUCCEEDED(ptmSysmInit()))
	{
		PTMSYSM_ConfigureNew3DSCPU(__ctru_speedup ? 3 : 0);
		ptmSysmExit();
	}
}

void osSetSpeedupEnable(bool enable)
{
	__ctru_speedup = enable;
	__ctru_speedup_config();
}
