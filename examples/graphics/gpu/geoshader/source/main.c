/*
 * ~~ Simple libctru GPU geometry shader example ~~
 * This example demonstrates the basics of using the PICA200 in a 3DS homebrew
 * application in order to render a basic scene using a geoshader.
 * The example geoshader receives the vertices of a triangle and emits three
 * smaller triangles, thus forming a 'triforce' shape.
 */

#include "gpu.h"
#include "vshader_shbin.h"
#include "gshader_shbin.h"

#define CLEAR_COLOR 0x68B0D8FF

typedef struct { float position[3]; float color[4]; } vertex;

static const vertex vertex_list[] =
{
	{ {200.0f, 200.0f, 0.5f}, {1.0f, 0.0f, 0.0f, 1.0f} },
	{ {100.0f,  40.0f, 0.5f}, {0.0f, 1.0f, 0.0f, 1.0f} },
	{ {300.0f,  40.0f, 0.5f}, {0.0f, 0.0f, 1.0f, 1.0f} },
};

#define vertex_list_count (sizeof(vertex_list)/sizeof(vertex_list[0]))

static DVLB_s *vshader_dvlb, *gshader_dvlb;
static shaderProgram_s program;
static int uLoc_projection;
static matrix_4x4 projection;

static void* vbo_data;

static void sceneInit(void)
{
	// Load the shaders and create a shader program
	// The geoshader stride is set to 6 so that it processes a triangle at a time
	vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
	gshader_dvlb = DVLB_ParseFile((u32*)gshader_shbin, gshader_shbin_size);
	shaderProgramInit(&program);
	shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
	shaderProgramSetGsh(&program, &gshader_dvlb->DVLE[0], 6);

	// Get the location of the projection matrix uniform
	uLoc_projection = shaderInstanceGetUniformLocation(program.geometryShader, "projection");

	// Compute the projection matrix
	m4x4_ortho_tilt(&projection, 0.0, 400.0, 0.0, 240.0, 0.0, 1.0);

	// Create the VBO (vertex buffer object)
	vbo_data = linearAlloc(sizeof(vertex_list));
	memcpy(vbo_data, vertex_list, sizeof(vertex_list));
}

static void sceneRender(void)
{
	// Bind the shader program
	shaderProgramUse(&program);

	// Configure the first fragment shading substage to just pass through the vertex color
	// See https://www.opengl.org/sdk/docs/man2/xhtml/glTexEnv.xml for more insight
	GPU_SetTexEnv(0,
		GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR), // RGB channels
		GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR), // Alpha
		GPU_TEVOPERANDS(0, 0, 0), // RGB
		GPU_TEVOPERANDS(0, 0, 0), // Alpha
		GPU_REPLACE, GPU_REPLACE, // RGB, Alpha
		0xFFFFFFFF);

	// Configure the "attribute buffers" (that is, the vertex input buffers)
	GPU_SetAttributeBuffers(
		2, // Number of inputs per vertex
		(u32*)osConvertVirtToPhys((u32)vbo_data), // Location of the VBO
		GPU_ATTRIBFMT(0, 3, GPU_FLOAT) |
		GPU_ATTRIBFMT(1, 4, GPU_FLOAT), // Format of the inputs (in this case the only input is a 3-element float vector)
		0xFFC, // Unused attribute mask, in our case bit 0 is cleared since it is used
		0x10, // Attribute permutations (here it is the identity)
		1, // Number of buffers
		(u32[]) { 0x0 }, // Buffer offsets (placeholders)
		(u64[]) { 0x10 }, // Attribute permutations for each buffer (identity again)
		(u8[])  { 2 }); // Number of attributes for each buffer

	// Upload the projection matrix
	GPU_SetFloatUniformMatrix(GPU_GEOMETRY_SHADER, uLoc_projection, &projection);

	// Draw the VBO - GPU_GEOMETRY_PRIM allows the geoshader to control primitive emission
	GPU_DrawArray(GPU_GEOMETRY_PRIM, 0, vertex_list_count);
}

static void sceneExit(void)
{
	// Free the VBO
	linearFree(vbo_data);

	// Free the shader program
	shaderProgramFree(&program);
	DVLB_Free(vshader_dvlb);
	DVLB_Free(gshader_dvlb);
}

int main()
{
	// Initialize graphics
	gfxInitDefault();
	gpuInit();

	// Initialize the scene
	sceneInit();
	gpuClearBuffers(CLEAR_COLOR);

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();  // Synchronize with the start of VBlank
		gfxSwapBuffersGpu(); // Swap the framebuffers so that the frame that we rendered last frame is now visible
		hidScanInput();      // Read the user input

		// Respond to user input
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		// Render the scene
		gpuFrameBegin();
		sceneRender();
		gpuFrameEnd();
		gpuClearBuffers(CLEAR_COLOR);

		// Flush the framebuffers out of the data cache (not necessary with pure GPU rendering)
		//gfxFlushBuffers();
	}

	// Deinitialize the scene
	sceneExit();

	// Deinitialize graphics
	gpuExit();
	gfxExit();
	return 0;
}
