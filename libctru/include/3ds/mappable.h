#pragma once

// Functions for allocating/deallocating mappable memory
void* mappableAlloc(size_t size); // returns a page-aligned address
void mappableFree(void* mem);
u32 mappableSpaceFree(); // get free mappable space in bytes
