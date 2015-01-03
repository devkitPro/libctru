#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/gpu/shaderProgram.h>

Result shaderInstanceInit(shaderInstance_s* si, DVLE_s* dvle)
{
	if(!si || !dvle)return -1;

	si->dvle = dvle;
	si->boolUniforms = 0xFFFF;

	return 0;
}

Result shaderInstanceFree(shaderInstance_s* si)
{
	if(!si)return -1;

	free(si);

	return 0;
}

Result shaderInstanceSetBool(shaderInstance_s* si, int id, bool value)
{
	if(!si)return -1;
	if(id<0 || id>15)return -2;

	si->boolUniforms &= ~(1<<id);
	si->boolUniforms |= (!value)<<id;

	return 0;
}

Result shaderInstanceGetBool(shaderInstance_s* si, int id, bool* value)
{
	if(!si)return -1;
	if(id<0 || id>15)return -2;
	if(!value)return -3;

	*value = !((si->boolUniforms>>id)&1);

	return 0;
}

Result shaderProgramInit(shaderProgram_s* sp)
{
	if(!sp)return -1;

	sp->vertexShader = NULL;
	sp->geometryShader = NULL;

	return 0;
}

Result shaderProgramFree(shaderProgram_s* sp)
{
	if(!sp)return -1;

	shaderInstanceFree(sp->vertexShader);
	shaderInstanceFree(sp->geometryShader);

	free(sp);

	return 0;
}

Result shaderProgramSetVsh(shaderProgram_s* sp, DVLE_s* dvle)
{
	if(!sp || !dvle)return -1;
	if(dvle->type != VERTEX_SHDR)return -2;

	if(sp->vertexShader)shaderInstanceFree(sp->vertexShader);

	sp->vertexShader = (shaderInstance_s*)malloc(sizeof(shaderInstance_s));
	if(!sp->vertexShader)return -3;

	return shaderInstanceInit(sp->vertexShader, dvle);
}

Result shaderProgramSetGsh(shaderProgram_s* sp, DVLE_s* dvle)
{
	if(!sp || !dvle)return -1;
	if(dvle->type != GEOMETRY_SHDR)return -2;

	if(sp->geometryShader)shaderInstanceFree(sp->geometryShader);

	sp->geometryShader = (shaderInstance_s*)malloc(sizeof(shaderInstance_s));
	if(!sp->geometryShader)return -3;

	return shaderInstanceInit(sp->geometryShader, dvle);
}

Result shaderProgramUse(shaderProgram_s* sp)
{
	if(!sp)return -1;

	if(!sp->vertexShader)return -2;

	if(!sp->geometryShader)
	{
		// only deal with vertex shader
	}else{
		// setup both vertex and geometry shader
	}

	return 0;
}
