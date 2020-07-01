#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/os.h>
#include <3ds/svc.h>
#include <3ds/synchronization.h>
#include <3ds/services/ptmsysm.h>

#include <sys/time.h>
#include <reent.h>
#include <unistd.h>

// Work around the VFP not supporting 64-bit integer <--> floating point conversion
static inline double u64_to_double(u64 value) {
	return (((double)(u32)(value >> 32))*0x100000000ULL+(u32)value);
}

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
osTimeRef_s osGetTimeRef(void) {
//---------------------------------------------------------------------------------
	osTimeRef_s tr;
	u32 next = OS_SharedConfig->timeref_cnt;
	u32 cur;

	do {
		cur = next;
		tr = OS_SharedConfig->timeref[cur&1];
		__dmb();
		next = OS_SharedConfig->timeref_cnt;
	} while (cur != next);

	return tr;
}

//---------------------------------------------------------------------------------
u64 osGetTime(void) {
//---------------------------------------------------------------------------------
	// Read the latest time reference point published by PTM
	osTimeRef_s tr = osGetTimeRef();

	// Calculate the time elapsed since the reference point using the system clock
	s64 elapsed_tick = svcGetSystemTick() - tr.value_tick;
	s64 elapsed_ms = elapsed_tick * 1000 / tr.sysclock_hz;

	// Apply the drift adjustment if present:
	// Every time PTM publishes a new reference point it measures by how long the
	// system clock has drifted with respect to the RTC. It also recalculates the
	// system clock frequency using RTC data in order to minimize future drift.
	// The idea behind the following logic is to reapply the inaccuracy to the
	// calculated timestamp, but gradually reducing it to zero over the course of
	// an hour. This ensures the monotonic growth of the returned time value.
	const s64 hour_in_ms = 60*60*1000; // milliseconds in an hour
	if (tr.drift_ms != 0 && elapsed_ms < hour_in_ms)
		elapsed_ms += tr.drift_ms * (hour_in_ms - elapsed_ms) / hour_in_ms;

	// Return the final timestamp
	return tr.value_ms + elapsed_ms;
}

//---------------------------------------------------------------------------------
int __libctru_gtod(struct _reent *ptr, struct timeval *tp, struct timezone *tz) {
//---------------------------------------------------------------------------------
	if (tp != NULL) {
		// Retrieve current time, adjusting epoch from 1900 to 1970
		s64 now = osGetTime() - 2208988800000ULL;

		// Convert to struct timeval
		tp->tv_sec = now / 1000;
		tp->tv_usec = (now - 1000*tp->tv_sec) * 1000;
	}

	if (tz != NULL) {
		// Provide dummy information, as the 3DS does not have the concept of timezones
		tz->tz_minuteswest = 0;
		tz->tz_dsttime = 0;
	}

	return 0;
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
