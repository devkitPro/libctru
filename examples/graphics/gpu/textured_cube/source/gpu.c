#include "gpu.h"

#define DISPLAY_TRANSFER_FLAGS \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
	GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
	GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

static u32 *colorBuf, *depthBuf;
static u32 *cmdBuf;

void gpuInit(void)
{
	colorBuf = vramAlloc(400*240*4);
	depthBuf = vramAlloc(400*240*4);
	cmdBuf = linearAlloc(0x40000*4);

	GPU_Init(NULL);
	GPU_Reset(NULL, cmdBuf, 0x40000);
}

void gpuExit(void)
{
	linearFree(cmdBuf);
	vramFree(depthBuf);
	vramFree(colorBuf);
}

void gpuClearBuffers(u32 clearColor)
{
	GX_MemoryFill(
		colorBuf, clearColor, &colorBuf[240*400], GX_FILL_TRIGGER | GX_FILL_32BIT_DEPTH,
		depthBuf, 0,          &depthBuf[240*400], GX_FILL_TRIGGER | GX_FILL_32BIT_DEPTH);
	gspWaitForPSC0(); // Wait for the fill to complete
}

void gpuFrameBegin(void)
{
	// Configure the viewport and the depth linear conversion function
	GPU_SetViewport(
		(u32*)osConvertVirtToPhys((u32)depthBuf),
		(u32*)osConvertVirtToPhys((u32)colorBuf),
		0, 0, 240, 400); // The top screen is physically 240x400 pixels
	GPU_DepthMap(-1.0f, 0.0f); // calculate the depth value from the Z coordinate in the following way: -1.0*z + 0.0

	// Configure some boilerplate
	GPU_SetFaceCulling(GPU_CULL_BACK_CCW);
	GPU_SetStencilTest(false, GPU_ALWAYS, 0x00, 0xFF, 0x00);
	GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, GPU_STENCIL_KEEP);
	GPU_SetBlendingColor(0,0,0,0);
	GPU_SetDepthTestAndWriteMask(true, GPU_GREATER, GPU_WRITE_ALL);

	// This is unknown
	GPUCMD_AddMaskedWrite(GPUREG_0062, 0x1, 0);
	GPUCMD_AddWrite(GPUREG_0118, 0);

	// Configure alpha blending and test
	GPU_SetAlphaBlending(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
	GPU_SetAlphaTest(false, GPU_ALWAYS, 0x00);

	int i;
	for (i = 0; i < 6; i ++)
		GPU_SetDummyTexEnv(i);
}

void gpuFrameEnd(void)
{
	// Finish rendering
	GPU_FinishDrawing();
	GPUCMD_Finalize();
	GPUCMD_FlushAndRun();
	gspWaitForP3D(); // Wait for the rendering to complete

	// Transfer the GPU output to the framebuffer
	GX_DisplayTransfer(
		colorBuf, GX_BUFFER_DIM(240, 400),
		(u32*)gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), GX_BUFFER_DIM(240, 400),
		DISPLAY_TRANSFER_FLAGS);
	gspWaitForPPF(); // Wait for the transfer to complete

	// Reset the command buffer
	GPUCMD_SetBufferOffset(0);
};

void GPU_SetDummyTexEnv(int id)
{
	GPU_SetTexEnv(id,
		GPU_TEVSOURCES(GPU_PREVIOUS, 0, 0),
		GPU_TEVSOURCES(GPU_PREVIOUS, 0, 0),
		GPU_TEVOPERANDS(0, 0, 0),
		GPU_TEVOPERANDS(0, 0, 0),
		GPU_REPLACE,
		GPU_REPLACE,
		0xFFFFFFFF);
}
