#ifndef TYPES_H
#define TYPES_H

	#include <stdint.h>
	#include <stdbool.h>

	#define U64_MAX (0xFFFFFFFFFFFFFFFF)

	typedef unsigned char u8;
	typedef unsigned short u16;
	typedef unsigned int u32;
	typedef unsigned long long u64;

	typedef signed char s8;
	typedef signed short s16;
	typedef signed int s32;
	typedef signed long long s64;

	typedef u16 wchar;

	typedef u32 Handle;
	typedef s32 Result;

#endif
