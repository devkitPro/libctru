#include <3ds.h>

#define TICKS_PER_MSEC 268123.480

typedef struct {
	u64 date_time;
	u64 update_tick;
	//...
} datetime_t;

static volatile u32* __datetime_selector =
	(u32*) 0x1FF81000;
static volatile datetime_t* __datetime1 =
	(datetime_t*) 0x1FF81020;
static volatile datetime_t* __datetime2 =
	(datetime_t*) 0x1FF81040;


u32 osConvertVirtToPhys(u32 vaddr)
{
	if(vaddr >= 0x14000000 && vaddr < 0x1c000000)
		return vaddr + 0x0c000000; // LINEAR heap
	if(vaddr >= 0x1F000000 && vaddr < 0x1F600000)
		return vaddr - 0x07000000; // VRAM
	if(vaddr >= 0x1FF00000 && vaddr < 0x1FF80000)
		return vaddr + 0x00000000; // DSP memory
	if(vaddr >= 0x30000000 && vaddr < 0x40000000)
		return vaddr - 0x10000000; // Only available under FIRM v8+ for certain processes.
	return 0;
}

// Returns number of milliseconds since 1st Jan 1900 00:00.
u64 osGetTime() {
	volatile datetime_t* dt;

	switch(*__datetime_selector & 1) {
	case 0:
		dt = __datetime1;
	case 1:
		dt = __datetime2;
	}

	u64 offset = (svcGetSystemTick() - dt->update_tick) / TICKS_PER_MSEC;
	return dt->date_time + offset;
}

u32 osGetFirmVersion() {
	return (*(u32*)0x1FF80000) & ~0xFF;
}

u32 osGetKernelVersion() {
	return (*(u32*)0x1FF80060) & ~0xFF;
}

const char* osStrError(u32 error) {
	switch((error>>26) & 0x3F) {
	case 0:
		return "Success.";
	case 1:
		return "Nothing happened.";
	case 2:
		return "Would block.";
	case 3:
		return "Not enough resources.";
	case 4:
		return "Not found.";
	case 5:
		return "Invalid state.";
	case 6:
		return "Unsupported.";
	case 7:
		return "Invalid argument.";
	case 8:
		return "Wrong argument.";
	case 9:
		return "Interrupted.";
	case 10:
		return "Internal error.";
	default:
		return "Unknown.";
	}
}
