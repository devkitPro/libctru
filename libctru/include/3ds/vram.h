#pragma once

// Functions for allocating/deallocating VRAM
void* vramAlloc(size_t size); // returns a 16-byte aligned address
void* vramMemAlign(size_t size, size_t alignment);
void* vramRealloc(void* mem, size_t size); // not implemented yet
void vramFree(void* mem);
u32 vramSpaceFree(); // get free VRAM space in bytes
