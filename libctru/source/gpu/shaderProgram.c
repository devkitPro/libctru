#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/gpu/registers.h>
#include <3ds/gpu/shaderProgram.h>

static void GPU_SetShaderOutmap(const u32 outmapData[8]);
static void GPU_SendShaderCode(GPU_SHADER_TYPE type, u32* data, u16 offset, u16 length);
static void GPU_SendOperandDescriptors(GPU_SHADER_TYPE type, u32* data, u16 offset, u16 length);

Result shaderInstanceInit(shaderInstance_s* si, DVLE_s* dvle)
{
	if(!si || !dvle)return -1;

	si->dvle = dvle;

	si->boolUniforms = 0;
	si->boolUniformMask = 0;
	si->intUniforms[0] = 0x00000000;
	si->intUniforms[1] = 0x00000000;
	si->intUniforms[2] = 0x00000000;
	si->intUniforms[3] = 0x00000000;
	si->float24Uniforms = NULL;
	si->intUniformMask = 0;

	int i;
	DVLE_constEntry_s* cnst = dvle->constTableData;
	if(cnst)
	{
		int float24cnt=0;
		for(i=0; i<dvle->constTableSize; i++)
		{
			switch(cnst[i].type)
			{
				case DVLE_CONST_BOOL:
					shaderInstanceSetBool(si, cnst[i].id, cnst[i].data[0]&1);
					break;
				case DVLE_CONST_u8:
					if(cnst[i].id<4)
					{
						si->intUniforms[cnst[i].id] = cnst[i].data[0];
						si->intUniformMask |= (1<<cnst[i].id);
					}
					break;
				case DVLE_CONST_FLOAT24:
					float24cnt++;
					break;
			}
		}

		if(float24cnt)
		{
			si->float24Uniforms = malloc(sizeof(float24Uniform_s)*float24cnt);
			if(si->float24Uniforms)
			{
				float24cnt = 0;
				u32 rev[3];
				u8* rev8=(u8*)rev;
				for(i=0; i<dvle->constTableSize; i++)
				{
					if(cnst[i].type==DVLE_CONST_FLOAT24)
					{
						memcpy(&rev8[0], &cnst[i].data[0], 3);
						memcpy(&rev8[3], &cnst[i].data[1], 3);
						memcpy(&rev8[6], &cnst[i].data[2], 3);
						memcpy(&rev8[9], &cnst[i].data[3], 3);

						si->float24Uniforms[float24cnt].id = cnst[i].id&0xFF;
						si->float24Uniforms[float24cnt].data[0] = rev[2];
						si->float24Uniforms[float24cnt].data[1] = rev[1];
						si->float24Uniforms[float24cnt].data[2] = rev[0];

						float24cnt++;
					}
				}
			}
			si->numFloat24Uniforms = float24cnt;
		}
	}

	return 0;
}

Result shaderInstanceFree(shaderInstance_s* si)
{
	if(!si)return -1;

	if(si->float24Uniforms)free(si->float24Uniforms);
	free(si);

	return 0;
}

Result shaderInstanceSetBool(shaderInstance_s* si, int id, bool value)
{
	if(!si)return -1;
	if(id<0 || id>15)return -2;

	si->boolUniforms &= ~(1<<id);
	si->boolUniforms |= (value)<<id;
	si->boolUniformMask |= (1<<id);

	return 0;
}

Result shaderInstanceGetBool(shaderInstance_s* si, int id, bool* value)
{
	if(!si)return -1;
	if(id<0 || id>15)return -2;
	if(!value)return -3;

	*value = ((si->boolUniforms>>id)&1);

	return 0;
}

s8 shaderInstanceGetUniformLocation(shaderInstance_s* si, const char* name)
{
	if(!si)return -1;

	return DVLE_GetUniformRegister(si->dvle, name);
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

Result shaderProgramSetGsh(shaderProgram_s* sp, DVLE_s* dvle, u8 stride)
{
	if(!sp || !dvle)return -1;
	if(dvle->type != GEOMETRY_SHDR)return -2;

	if(sp->geometryShader)shaderInstanceFree(sp->geometryShader);

	sp->geometryShader = (shaderInstance_s*)malloc(sizeof(shaderInstance_s));
	if(!sp->geometryShader)return -3;

	sp->geoShaderInputPermutation[0] = 0x76543210;
	sp->geoShaderInputPermutation[1] = 0xFEDCBA98;
	sp->geoShaderInputStride = stride;

	return shaderInstanceInit(sp->geometryShader, dvle);
}

Result shaderProgramSetGshInputPermutation(shaderProgram_s* sp, u64 permutation)
{
	if(!sp || !sp->geometryShader)return -1;

	sp->geoShaderInputPermutation[0] = permutation & 0xFFFFFFFF;
	sp->geoShaderInputPermutation[1] = permutation>>32;
	return 0;
}

static inline void shaderProgramUploadDvle(const DVLE_s* dvle)
{
	const DVLP_s* dvlp = dvle->dvlp;
	// Limit vertex shader code size to the first 512 instructions
	int codeSize = dvle->type == GEOMETRY_SHDR ? dvlp->codeSize : (dvlp->codeSize < 512 ? dvlp->codeSize : 512);
	GPU_SendShaderCode(dvle->type, dvlp->codeData, 0, codeSize);
	GPU_SendOperandDescriptors(dvle->type, dvlp->opcdescData, 0, dvlp->opdescSize);
}

static inline void shaderProgramMergeOutmaps(u32* outmapData, const u32* vshOutmap, const u32* gshOutmap)
{
	int i, j;

	// Find and copy attributes common to both vertex and geometry shader
	u32 vsh_common = 0, gsh_common = 0;
	for (i = 1; i < 8; i ++)
	{
		u32 mask = gshOutmap[i];
		if (mask == 0x1F1F1F1F)
			break;
		for (j = 1; j < 8; j ++)
		{
			if (vshOutmap[j] == mask)
			{
				outmapData[++outmapData[0]] = mask;
				vsh_common |= BIT(j);
				gsh_common |= BIT(i);
				break;
			}
		}
	}

	// Find and copy attributes that are exclusive to the geometry shader
	for (i = 1; i < 8; i ++)
	{
		u32 mask = gshOutmap[i];
		if (mask == 0x1F1F1F1F)
			break;
		if (!(gsh_common & BIT(i)))
			outmapData[++outmapData[0]] = mask;
	}

	// Find and copy attributes that are exclusive to the vertex shader
	for (i = 1; i < 8; i ++)
	{
		u32 mask = vshOutmap[i];
		if (mask == 0x1F1F1F1F)
			break;
		if (!(vsh_common & BIT(i)))
			outmapData[++outmapData[0]] = mask;
	}
}

Result shaderProgramConfigure(shaderProgram_s* sp, bool sendVshCode, bool sendGshCode)
{
	if (!sp || !sp->vertexShader) return -1;

	// Get pointers to relevant structures
	const DVLE_s* vshDvle = sp->vertexShader->dvle;
	const DVLE_s* gshDvle = sp->geometryShader ? sp->geometryShader->dvle : NULL;
	const DVLE_s* mainDvle = gshDvle ? gshDvle : vshDvle;

	// Variables for working with the outmap
	u32 outmapData[8];
	u32 outmapMode = mainDvle->outmapMode;
	u32 outmapClock = mainDvle->outmapClock;

	// Initialize geometry engine - do this early in order to ensure all 4 units are correctly initialized
	GPUCMD_AddMaskedWrite(GPUREG_GEOSTAGE_CONFIG, 0x3, gshDvle ? 2 : 0);
	GPUCMD_AddMaskedWrite(GPUREG_GEOSTAGE_CONFIG2, 0x3, 0);
	GPUCMD_AddMaskedWrite(GPUREG_VSH_COM_MODE, 0x1, gshDvle ? 1 : 0);

	// Set up vertex shader code blob (if necessary)
	if (sendVshCode)
		shaderProgramUploadDvle(vshDvle);

	// Set up vertex shader entrypoint & outmap mask
	GPUCMD_AddWrite(GPUREG_VSH_ENTRYPOINT, 0x7FFF0000|(vshDvle->mainOffset&0xFFFF));
	GPUCMD_AddWrite(GPUREG_VSH_OUTMAP_MASK, vshDvle->outmapMask);
	GPUCMD_AddWrite(GPUREG_VSH_OUTMAP_TOTAL1, vshDvle->outmapData[0]-1);
	GPUCMD_AddWrite(GPUREG_VSH_OUTMAP_TOTAL2, vshDvle->outmapData[0]-1);

	// Set up geometry shader (if present)
	if (gshDvle)
	{
		// Set up geometry shader code blob (if necessary)
		if (sendGshCode)
			shaderProgramUploadDvle(gshDvle);

		// Set up geometry shader entrypoint & outmap mask
		GPUCMD_AddWrite(GPUREG_GSH_ENTRYPOINT, 0x7FFF0000|(gshDvle->mainOffset&0xFFFF));
		GPUCMD_AddWrite(GPUREG_GSH_OUTMAP_MASK, gshDvle->outmapMask);
	}

	// Merge vertex shader & geometry shader outmaps if requested
	if (gshDvle && gshDvle->mergeOutmaps)
	{
		// Clear outmap
		memset(outmapData, 0x1F, sizeof(outmapData));
		outmapData[0] = 0;

		// Merge outmaps
		shaderProgramMergeOutmaps(outmapData, vshDvle->outmapData, gshDvle->outmapData);
		outmapMode  |= vshDvle->outmapMode;
		outmapClock |= vshDvle->outmapClock;
	} else
		memcpy(outmapData, mainDvle->outmapData, sizeof(outmapData));

	// Upload and configure outmap
	GPU_SetShaderOutmap(outmapData);
	GPUCMD_AddWrite(GPUREG_SH_OUTATTR_MODE, outmapMode);
	GPUCMD_AddWrite(GPUREG_SH_OUTATTR_CLOCK, outmapClock);

	// Configure geostage
	if (gshDvle)
	{
		// Input stride: use value if specified, otherwise use number of outputs in vertex shader
		int stride = sp->geoShaderInputStride ? sp->geoShaderInputStride : vshDvle->outmapData[0];

		// Enable or disable variable-size primitive processing
		GPUCMD_AddMaskedWrite(GPUREG_GEOSTAGE_CONFIG, 0xA, gshDvle->gshMode == GSH_VARIABLE_PRIM ? 0x80000000 : 0);

		// Set up geoshader processing mode
		u32 misc = gshDvle->gshMode;
		if (misc == GSH_FIXED_PRIM)
			misc |= 0x01000000 | ((u32)gshDvle->gshFixedVtxStart<<16) | ((stride-1)<<12) | ((u32)(gshDvle->gshFixedVtxNum-1)<<8);
		GPUCMD_AddWrite(GPUREG_GSH_MISC0, misc);

		// Set up variable-size primitive mode parameters
		GPUCMD_AddWrite(GPUREG_GSH_MISC1, gshDvle->gshMode == GSH_VARIABLE_PRIM ? (gshDvle->gshVariableVtxNum-1) : 0);

		// Set up geoshader input
		GPUCMD_AddWrite(GPUREG_GSH_INPUTBUFFER_CONFIG, 0x08000000 | (gshDvle->gshMode ? 0x0100 : 0) | (stride-1));

		// Set up geoshader permutation
		GPUCMD_AddIncrementalWrites(GPUREG_GSH_ATTRIBUTES_PERMUTATION_LOW, sp->geoShaderInputPermutation, 2);
	} else
	{
		// Defaults for when geostage is disabled
		GPUCMD_AddMaskedWrite(GPUREG_GEOSTAGE_CONFIG, 0xA, 0);
		GPUCMD_AddWrite(GPUREG_GSH_MISC0, 0);
		GPUCMD_AddWrite(GPUREG_GSH_MISC1, 0);
		GPUCMD_AddWrite(GPUREG_GSH_INPUTBUFFER_CONFIG, 0xA0000000);
	}

	return 0;
}

Result shaderProgramUse(shaderProgram_s* sp)
{
	Result rc = shaderProgramConfigure(sp, true, true);
	if (R_FAILED(rc)) return rc;

	int i;

	// Set up uniforms
	GPUCMD_AddWrite(GPUREG_VSH_BOOLUNIFORM, 0x7FFF0000|sp->vertexShader->boolUniforms);
	GPUCMD_AddIncrementalWrites(GPUREG_VSH_INTUNIFORM_I0, sp->vertexShader->intUniforms, 4);
	for(i=0; i<sp->vertexShader->numFloat24Uniforms; i++) GPUCMD_AddIncrementalWrites(GPUREG_VSH_FLOATUNIFORM_CONFIG, (u32*)&sp->vertexShader->float24Uniforms[i], 4);
	if (sp->geometryShader)
	{
		GPUCMD_AddWrite(GPUREG_GSH_BOOLUNIFORM, 0x7FFF0000|sp->geometryShader->boolUniforms);
		GPUCMD_AddIncrementalWrites(GPUREG_GSH_INTUNIFORM_I0, sp->geometryShader->intUniforms, 4);
		for(i=0; i<sp->geometryShader->numFloat24Uniforms; i++) GPUCMD_AddIncrementalWrites(GPUREG_GSH_FLOATUNIFORM_CONFIG, (u32*)&sp->geometryShader->float24Uniforms[i], 4);
	}

	return 0;
}

void GPU_SetShaderOutmap(const u32 outmapData[8])
{
	GPUCMD_AddMaskedWrite(GPUREG_PRIMITIVE_CONFIG, 0x1, outmapData[0]-1);
	GPUCMD_AddIncrementalWrites(GPUREG_SH_OUTMAP_TOTAL, outmapData, 8);
}

void GPU_SendShaderCode(GPU_SHADER_TYPE type, u32* data, u16 offset, u16 length)
{
	if(!data)return;

	int regOffset=(type==GPU_GEOMETRY_SHADER)?(-0x30):(0x0);

	GPUCMD_AddWrite(GPUREG_VSH_CODETRANSFER_CONFIG+regOffset, offset);

	int i;
	for(i=0;i<length;i+=0x80)GPUCMD_AddWrites(GPUREG_VSH_CODETRANSFER_DATA+regOffset, &data[i], ((length-i)<0x80)?(length-i):0x80);

	GPUCMD_AddWrite(GPUREG_VSH_CODETRANSFER_END+regOffset, 0x00000001);
}

void GPU_SendOperandDescriptors(GPU_SHADER_TYPE type, u32* data, u16 offset, u16 length)
{
	if(!data)return;

	int regOffset=(type==GPU_GEOMETRY_SHADER)?(-0x30):(0x0);

	GPUCMD_AddWrite(GPUREG_VSH_OPDESCS_CONFIG+regOffset, offset);

	GPUCMD_AddWrites(GPUREG_VSH_OPDESCS_DATA+regOffset, data, length);
}
