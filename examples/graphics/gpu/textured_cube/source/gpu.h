/*
 * Bare-bones simplistic GPU wrapper
 * This library is common to all libctru GPU examples
 */

#pragma once
#include <string.h>
#include <3ds.h>
#include "3dmath.h"

void gpuInit(void);
void gpuExit(void);

void gpuClearBuffers(u32 clearColor);

void gpuFrameBegin(void);
void gpuFrameEnd(void);

// Configures the specified fixed-function fragment shading substage to be a no-operation
void GPU_SetDummyTexEnv(int id);

// Uploads an uniform matrix
static inline void GPU_SetFloatUniformMatrix(GPU_SHADER_TYPE type, int location, matrix_4x4* matrix)
{
	GPU_SetFloatUniform(type, location, (u32*)matrix, 4);
}
