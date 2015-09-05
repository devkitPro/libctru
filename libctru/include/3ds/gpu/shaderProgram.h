#pragma once

#include <3ds/types.h>
#include <3ds/gpu/shbin.h>

typedef struct
{
	u32 id;
	u32 data[3];
}float24Uniform_s;

// this structure describes an instance of either a vertex or geometry shader
typedef struct
{
	DVLE_s* dvle;
	u16 boolUniforms;
	u32 intUniforms[4];
	float24Uniform_s* float24Uniforms;
	u8 numFloat24Uniforms;
}shaderInstance_s;

// this structure describes an instance of a full shader program
typedef struct
{
	shaderInstance_s* vertexShader;
	shaderInstance_s* geometryShader;
	u8 geometryShaderInputStride;
}shaderProgram_s;

Result shaderInstanceInit(shaderInstance_s* si, DVLE_s* dvle);
Result shaderInstanceFree(shaderInstance_s* si);
Result shaderInstanceSetBool(shaderInstance_s* si, int id, bool value);
Result shaderInstanceGetBool(shaderInstance_s* si, int id, bool* value);
Result shaderInstanceGetUniformLocation(shaderInstance_s* si, const char* name);

Result shaderProgramInit(shaderProgram_s* sp);
Result shaderProgramFree(shaderProgram_s* sp);
Result shaderProgramSetVsh(shaderProgram_s* sp, DVLE_s* dvle);
Result shaderProgramSetGsh(shaderProgram_s* sp, DVLE_s* dvle, u8 stride);
Result shaderProgramUse(shaderProgram_s* sp);
