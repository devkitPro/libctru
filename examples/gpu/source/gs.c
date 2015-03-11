#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <3ds.h>

#include "gs.h"
#include "math.h"

#define BUFFERMATRIXLIST_SIZE (GS_MATRIXSTACK_SIZE*4)

static void gsInitMatrixStack();

Handle linearAllocMutex;

static u32 gsMatrixStackRegisters[GS_MATRIXTYPES];

typedef struct
{
	u32 offset;
	mtx44 data;
}bufferMatrix_s;

bufferMatrix_s bufferMatrixList[BUFFERMATRIXLIST_SIZE];
int bufferMatrixListLength;

//----------------------
//   GS SYSTEM STUFF
//----------------------

void initBufferMatrixList()
{
	bufferMatrixListLength=0;
}

void gsInit(shaderProgram_s* shader)
{
	gsInitMatrixStack();
	initBufferMatrixList();
	svcCreateMutex(&linearAllocMutex, false);
	if(shader)
	{
		gsMatrixStackRegisters[0]=shaderInstanceGetUniformLocation(shader->vertexShader, "projection");
		gsMatrixStackRegisters[1]=shaderInstanceGetUniformLocation(shader->vertexShader, "modelview");
		shaderProgramUse(shader);
	}
}

void gsExit(void)
{
	svcCloseHandle(linearAllocMutex);
}

void gsStartFrame(void)
{
	GPUCMD_SetBufferOffset(0);
	initBufferMatrixList();
}

void* gsLinearAlloc(size_t size)
{
	void* ret=NULL;

	svcWaitSynchronization(linearAllocMutex, U64_MAX);
	ret=linearAlloc(size);
	svcReleaseMutex(linearAllocMutex);
	
	return ret;
}

void gsLinearFree(void* mem)
{
	svcWaitSynchronization(linearAllocMutex, U64_MAX);
	linearFree(mem);
	svcReleaseMutex(linearAllocMutex);
}

//----------------------
//  MATRIX STACK STUFF
//----------------------

static mtx44 gsMatrixStacks[GS_MATRIXTYPES][GS_MATRIXSTACK_SIZE];
static u32 gsMatrixStackRegisters[GS_MATRIXTYPES]={0x00, 0x04};
static u8 gsMatrixStackOffsets[GS_MATRIXTYPES];
static bool gsMatrixStackUpdated[GS_MATRIXTYPES];
static GS_MATRIX gsCurrentMatrixType;

static void gsInitMatrixStack()
{
	int i;
	for(i=0; i<GS_MATRIXTYPES; i++)
	{
		gsMatrixStackOffsets[i]=0;
		gsMatrixStackUpdated[i]=true;
		loadIdentity44((float*)gsMatrixStacks[i][0]);
	}
	gsCurrentMatrixType=GS_PROJECTION;
}

float* gsGetMatrix(GS_MATRIX m)
{
	if(m<0 || m>=GS_MATRIXTYPES)return NULL;
	
	return (float*)gsMatrixStacks[m][gsMatrixStackOffsets[m]];
}

int gsLoadMatrix(GS_MATRIX m, float* data)
{
	if(m<0 || m>=GS_MATRIXTYPES || !data)return -1;
	
	memcpy(gsGetMatrix(m), data, sizeof(mtx44));

	gsMatrixStackUpdated[m]=true;

	return 0;
}

int gsPushMatrix()
{
	const GS_MATRIX m=gsCurrentMatrixType;
	if(m<0 || m>=GS_MATRIXTYPES)return -1;
	if(gsMatrixStackOffsets[m]<0 || gsMatrixStackOffsets[m]>=GS_MATRIXSTACK_SIZE-1)return -1;

	float* cur=gsGetMatrix(m);
	gsMatrixStackOffsets[m]++;
	memcpy(gsGetMatrix(m), cur, sizeof(mtx44));

	return 0;
}

int gsPopMatrix()
{
	const GS_MATRIX m=gsCurrentMatrixType;
	if(m<0 || m>=GS_MATRIXTYPES)return -1;
	if(gsMatrixStackOffsets[m]<1 || gsMatrixStackOffsets[m]>=GS_MATRIXSTACK_SIZE)return -1;

	gsMatrixStackOffsets[m]--;

	gsMatrixStackUpdated[m]=true;

	return 0;
}

int gsMatrixMode(GS_MATRIX m)
{
	if(m<0 || m>=GS_MATRIXTYPES)return -1;

	gsCurrentMatrixType=m;

	return 0;
}

//------------------------
// MATRIX TRANSFORM STUFF
//------------------------

int gsMultMatrix(float* data)
{
	if(!data)return -1;
	
	mtx44 tmp;
	multMatrix44(gsGetMatrix(gsCurrentMatrixType), data, (float*)tmp);
	memcpy(gsGetMatrix(gsCurrentMatrixType), (float*)tmp, sizeof(mtx44));

	gsMatrixStackUpdated[gsCurrentMatrixType]=true;

	return 0;
}

void gsLoadIdentity()
{
	loadIdentity44(gsGetMatrix(gsCurrentMatrixType));
	gsMatrixStackUpdated[gsCurrentMatrixType]=true;
}

void gsProjectionMatrix(float fovy, float aspect, float near, float far)
{
	initProjectionMatrix(gsGetMatrix(gsCurrentMatrixType), fovy, aspect, near, far);
	gsMatrixStackUpdated[gsCurrentMatrixType]=true;
}

void gsRotateX(float x)
{
	rotateMatrixX(gsGetMatrix(gsCurrentMatrixType), x, false);
	gsMatrixStackUpdated[gsCurrentMatrixType]=true;
}

void gsRotateY(float y)
{
	rotateMatrixY(gsGetMatrix(gsCurrentMatrixType), y, false);
	gsMatrixStackUpdated[gsCurrentMatrixType]=true;
}

void gsRotateZ(float z)
{
	rotateMatrixZ(gsGetMatrix(gsCurrentMatrixType), z, false);
	gsMatrixStackUpdated[gsCurrentMatrixType]=true;
}

void gsScale(float x, float y, float z)
{
	scaleMatrix(gsGetMatrix(gsCurrentMatrixType), x, y, z);
	gsMatrixStackUpdated[gsCurrentMatrixType]=true;
}

void gsTranslate(float x, float y, float z)
{
	translateMatrix(gsGetMatrix(gsCurrentMatrixType), x, y, z);
	gsMatrixStackUpdated[gsCurrentMatrixType]=true;
}

//----------------------
// MATRIX RENDER STUFF
//----------------------

static void gsSetUniformMatrix(u32 startreg, float* m)
{
	float param[16];

	param[0x0]=m[3]; //w
	param[0x1]=m[2]; //z
	param[0x2]=m[1]; //y
	param[0x3]=m[0]; //x

	param[0x4]=m[7];
	param[0x5]=m[6];
	param[0x6]=m[5];
	param[0x7]=m[4];
	
	param[0x8]=m[11];
	param[0x9]=m[10];
	param[0xa]=m[9];
	param[0xb]=m[8];

	param[0xc]=m[15];
	param[0xd]=m[14];
	param[0xe]=m[13];
	param[0xf]=m[12];

	GPU_SetFloatUniform(GPU_VERTEX_SHADER, startreg, (u32*)param, 4);
}

static int gsUpdateTransformation()
{
	GS_MATRIX m;
	for(m=0; m<GS_MATRIXTYPES; m++)
	{
		if(gsMatrixStackUpdated[m])
		{
			if(m==GS_PROJECTION && bufferMatrixListLength<BUFFERMATRIXLIST_SIZE)
			{
				GPUCMD_GetBuffer(NULL, NULL, &bufferMatrixList[bufferMatrixListLength].offset);
				memcpy(bufferMatrixList[bufferMatrixListLength].data, gsGetMatrix(m), sizeof(mtx44));
				bufferMatrixListLength++;
			}
			gsSetUniformMatrix(gsMatrixStackRegisters[m], gsGetMatrix(m));
			gsMatrixStackUpdated[m]=false;
		}
	}
	return 0;
}

void gsAdjustBufferMatrices(mtx44 transformation)
{
	int i;
	u32* buffer;
	u32 offset;
	GPUCMD_GetBuffer(&buffer, NULL, &offset);
	for(i=0; i<bufferMatrixListLength; i++)
	{
		u32 o=bufferMatrixList[i].offset;
		if(o+2<offset) //TODO : better check, need to account for param size
		{
			mtx44 newMatrix;
			GPUCMD_SetBufferOffset(o);
			multMatrix44((float*)bufferMatrixList[i].data, (float*)transformation, (float*)newMatrix);
			gsSetUniformMatrix(gsMatrixStackRegisters[GS_PROJECTION], (float*)newMatrix);
		}
	}
	GPUCMD_SetBufferOffset(offset);
}

//----------------------
//      VBO STUFF
//----------------------

int gsVboInit(gsVbo_s* vbo)
{
	if(!vbo)return -1;

	vbo->data=NULL;
	vbo->currentSize=0;
	vbo->maxSize=0;
	vbo->commands=NULL;
	vbo->commandsSize=0;

	return 0;
}

int gsVboCreate(gsVbo_s* vbo, u32 size)
{
	if(!vbo)return -1;

	vbo->data=gsLinearAlloc(size);
	vbo->numVertices=0;
	vbo->currentSize=0;
	vbo->maxSize=size;

	return 0;
}

void* gsVboGetOffset(gsVbo_s* vbo)
{
	if(!vbo)return NULL;

	return (void*)(&((u8*)vbo->data)[vbo->currentSize]);
}

int gsVboAddData(gsVbo_s* vbo, void* data, u32 size, u32 units)
{
	if(!vbo || !data || !size)return -1;
	if(((s32)vbo->maxSize)-((s32)vbo->currentSize) < size)return -1;

	memcpy(gsVboGetOffset(vbo), data, size);
	vbo->currentSize+=size;
	vbo->numVertices+=units;

	return 0;
}

int gsVboFlushData(gsVbo_s* vbo)
{
	if(!vbo)return -1;

	//unnecessary if we use flushAndRun
	// GSPGPU_FlushDataCache(NULL, vbo->data, vbo->currentSize);

	return 0;
}

int gsVboDestroy(gsVbo_s* vbo)
{
	if(!vbo)return -1;

	if(vbo->commands)free(vbo->commands);
	if(vbo->data)gsLinearFree(vbo->data);
	gsVboInit(vbo);

	return 0;
}

extern u32 debugValue[];

void GPU_DrawArrayDirectly(GPU_Primitive_t primitive, u8* data, u32 n)
{
	//set attribute buffer address
	GPUCMD_AddSingleParam(0x000F0200, (osConvertVirtToPhys((u32)data))>>3);
	//set primitive type
	GPUCMD_AddSingleParam(0x0002025E, primitive);
	GPUCMD_AddSingleParam(0x0002025F, 0x00000001);
	//index buffer not used for drawArrays but 0x000F0227 still required
	GPUCMD_AddSingleParam(0x000F0227, 0x80000000);
	//pass number of vertices
	GPUCMD_AddSingleParam(0x000F0228, n);

	GPUCMD_AddSingleParam(0x00010253, 0x00000001);

	GPUCMD_AddSingleParam(0x00010245, 0x00000000);
	GPUCMD_AddSingleParam(0x000F022E, 0x00000001);
	GPUCMD_AddSingleParam(0x00010245, 0x00000001);
	GPUCMD_AddSingleParam(0x000F0231, 0x00000001);

	// GPUCMD_AddSingleParam(0x000F0111, 0x00000001); //breaks stuff
}

//not thread safe
int gsVboPrecomputeCommands(gsVbo_s* vbo)
{
	if(!vbo || vbo->commands)return -1;

	static u32 tmpBuffer[128];

	u32* savedAdr; u32 savedSize, savedOffset;
	GPUCMD_GetBuffer(&savedAdr, &savedSize, &savedOffset);
	GPUCMD_SetBuffer(tmpBuffer, 128, 0);

	GPU_DrawArrayDirectly(GPU_TRIANGLES, vbo->data, vbo->numVertices);
	
	GPUCMD_GetBuffer(NULL, NULL, &vbo->commandsSize);
	vbo->commands=memalign(0x4, vbo->commandsSize*4);
	if(!vbo->commands)return -1;
	memcpy(vbo->commands, tmpBuffer, vbo->commandsSize*4);

	GPUCMD_SetBuffer(savedAdr, savedSize, savedOffset);

	return 0;
}

extern u32* gpuCmdBuf;
extern u32 gpuCmdBufSize;
extern u32 gpuCmdBufOffset;

void _vboMemcpy50(u32* dst, u32* src);

void _GPUCMD_AddRawCommands(u32* cmd, u32 size)
{
	if(!cmd || !size)return;

	if(size*4==0x50)_vboMemcpy50(&gpuCmdBuf[gpuCmdBufOffset], cmd);
	else memcpy(&gpuCmdBuf[gpuCmdBufOffset], cmd, size*4);
	gpuCmdBufOffset+=size;
}

int gsVboDraw(gsVbo_s* vbo)
{
	if(!vbo || !vbo->data || !vbo->currentSize || !vbo->maxSize)return -1;

	gsUpdateTransformation();

	gsVboPrecomputeCommands(vbo);

	// u64 val=svcGetSystemTick();
	if(vbo->commands)
	{
		_GPUCMD_AddRawCommands(vbo->commands, vbo->commandsSize);
	}else{
		GPU_DrawArrayDirectly(GPU_TRIANGLES, vbo->data, vbo->numVertices);
	}
	// debugValue[5]+=(u32)(svcGetSystemTick()-val);
	// debugValue[6]++;

	return 0;
}
