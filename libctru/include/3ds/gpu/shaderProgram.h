#pragma once

#include <3ds/types.h>
#include <3ds/gpu/shbin.h>

// this structure describes an instance of either a vertex or geometry shader
typedef struct
{
	DVLE_s* dvle;
	u16 boolUniforms;
}shaderInstance_s;

// this structure describes an instance of a full shader program
typedef struct
{
	shaderInstance_s* vertexShader;
	shaderInstance_s* geometryShader;
}shaderProgram_s;

Result shaderInstanceInit(shaderInstance_s* si, DVLE_s* dvle);
Result shaderInstanceFree(shaderInstance_s* si);
Result shaderInstanceSetBool(shaderInstance_s* si, int id, bool value);
Result shaderInstanceGetBool(shaderInstance_s* si, int id, bool* value);

Result shaderProgramInit(shaderProgram_s* sp);
Result shaderProgramFree(shaderProgram_s* sp);
Result shaderProgramSetVsh(shaderProgram_s* sp, DVLE_s* dvle);
Result shaderProgramSetGsh(shaderProgram_s* sp, DVLE_s* dvle);
Result shaderProgramUse(shaderProgram_s* sp);
