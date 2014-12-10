#pragma once

// Functions for allocating/deallocating memory from linear heap
void* linearAlloc(size_t size); // returns a 16-byte aligned address
void* linearMemAlign(size_t size, size_t alignment);
void* linearRealloc(void* mem, size_t size); // not implemented yet
void linearFree(void* mem);
u32 linearSpaceFree(); // get free linear space in bytes
