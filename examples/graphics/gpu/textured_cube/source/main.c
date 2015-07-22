/*
 * ~~ Simple libctru GPU textured cube example ~~
 * This example demonstrates the basics of using the PICA200 in a 3DS homebrew
 * application in order to render a basic scene consisting of a rotating
 * textured cube which is also shaded using a simple shading algorithm.
 * The shading algorithm is explained in the vertex shader source code.
 */

#include "gpu.h"
#include "vshader_shbin.h"
#include "kitten_bin.h"

#define CLEAR_COLOR 0x68B0D8FF

typedef struct { float position[3]; float texcoord[2]; float normal[3]; } vertex;

static const vertex vertex_list[] =
{
	// First face (PZ)
	// First triangle
	{ {-0.5f, -0.5f, +0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, +1.0f} },
	{ {+0.5f, -0.5f, +0.5f}, {1.0f, 0.0f}, {0.0f, 0.0f, +1.0f} },
	{ {+0.5f, +0.5f, +0.5f}, {1.0f, 1.0f}, {0.0f, 0.0f, +1.0f} },
	// Second triangle
	{ {+0.5f, +0.5f, +0.5f}, {1.0f, 1.0f}, {0.0f, 0.0f, +1.0f} },
	{ {-0.5f, +0.5f, +0.5f}, {0.0f, 1.0f}, {0.0f, 0.0f, +1.0f} },
	{ {-0.5f, -0.5f, +0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, +1.0f} },

	// Second face (MZ)
	// First triangle
	{ {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f} },
	{ {-0.5f, +0.5f, -0.5f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f} },
	{ {+0.5f, +0.5f, -0.5f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f} },
	// Second triangle
	{ {+0.5f, +0.5f, -0.5f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f} },
	{ {+0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f} },
	{ {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f} },

	// Third face (PX)
	// First triangle
	{ {+0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {+1.0f, 0.0f, 0.0f} },
	{ {+0.5f, +0.5f, -0.5f}, {1.0f, 0.0f}, {+1.0f, 0.0f, 0.0f} },
	{ {+0.5f, +0.5f, +0.5f}, {1.0f, 1.0f}, {+1.0f, 0.0f, 0.0f} },
	// Second triangle
	{ {+0.5f, +0.5f, +0.5f}, {1.0f, 1.0f}, {+1.0f, 0.0f, 0.0f} },
	{ {+0.5f, -0.5f, +0.5f}, {0.0f, 1.0f}, {+1.0f, 0.0f, 0.0f} },
	{ {+0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {+1.0f, 0.0f, 0.0f} },

	// Fourth face (MX)
	// First triangle
	{ {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} },
	{ {-0.5f, -0.5f, +0.5f}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} },
	{ {-0.5f, +0.5f, +0.5f}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} },
	// Second triangle
	{ {-0.5f, +0.5f, +0.5f}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} },
	{ {-0.5f, +0.5f, -0.5f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} },
	{ {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} },

	// Fifth face (PY)
	// First triangle
	{ {-0.5f, +0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, +1.0f, 0.0f} },
	{ {-0.5f, +0.5f, +0.5f}, {1.0f, 0.0f}, {0.0f, +1.0f, 0.0f} },
	{ {+0.5f, +0.5f, +0.5f}, {1.0f, 1.0f}, {0.0f, +1.0f, 0.0f} },
	// Second triangle
	{ {+0.5f, +0.5f, +0.5f}, {1.0f, 1.0f}, {0.0f, +1.0f, 0.0f} },
	{ {+0.5f, +0.5f, -0.5f}, {0.0f, 1.0f}, {0.0f, +1.0f, 0.0f} },
	{ {-0.5f, +0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, +1.0f, 0.0f} },

	// Sixth face (MY)
	// First triangle
	{ {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f} },
	{ {+0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}, {0.0f, -1.0f, 0.0f} },
	{ {+0.5f, -0.5f, +0.5f}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f} },
	// Second triangle
	{ {+0.5f, -0.5f, +0.5f}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f} },
	{ {-0.5f, -0.5f, +0.5f}, {0.0f, 1.0f}, {0.0f, -1.0f, 0.0f} },
	{ {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f} },
};

#define vertex_list_count (sizeof(vertex_list)/sizeof(vertex_list[0]))

static DVLB_s* vshader_dvlb;
static shaderProgram_s program;
static int uLoc_projection, uLoc_modelView;
static int uLoc_lightVec, uLoc_lightHalfVec, uLoc_lightClr, uLoc_material;
static matrix_4x4 projection;
static matrix_4x4 material =
{
	{
	{ { 0.0f, 0.2f, 0.2f, 0.2f } }, // Ambient
	{ { 0.0f, 0.4f, 0.4f, 0.4f } }, // Diffuse
	{ { 0.0f, 0.8f, 0.8f, 0.8f } }, // Specular
	{ { 1.0f, 0.0f, 0.0f, 0.0f } }, // Emission
	}
};

static void* vbo_data;
static void* tex_data;
static float angleX = 0.0, angleY = 0.0;

static void sceneInit(void)
{
	// Load the vertex shader and create a shader program
	vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
	shaderProgramInit(&program);
	shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);

	// Get the location of the uniforms
	uLoc_projection   = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
	uLoc_modelView    = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");
	uLoc_lightVec     = shaderInstanceGetUniformLocation(program.vertexShader, "lightVec");
	uLoc_lightHalfVec = shaderInstanceGetUniformLocation(program.vertexShader, "lightHalfVec");
	uLoc_lightClr     = shaderInstanceGetUniformLocation(program.vertexShader, "lightClr");
	uLoc_material     = shaderInstanceGetUniformLocation(program.vertexShader, "material");

	// Compute the projection matrix
	m4x4_persp_tilt(&projection, 80.0f*M_PI/180.0f, 400.0f/240.0f, 0.01f, 1000.0f);

	// Create the VBO (vertex buffer object)
	vbo_data = linearAlloc(sizeof(vertex_list));
	memcpy(vbo_data, vertex_list, sizeof(vertex_list));

	// Load the texture
	tex_data = linearAlloc(kitten_bin_size);
	memcpy(tex_data, kitten_bin, kitten_bin_size);
}

static void sceneRender(void)
{
	// Bind the shader program
	shaderProgramUse(&program);

	// Configure the first fragment shading substage to blend the texture color with
	// the vertex color (calculated by the vertex shader using a lighting algorithm)
	// See https://www.opengl.org/sdk/docs/man2/xhtml/glTexEnv.xml for more insight
	GPU_SetTexEnv(0,
		GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR), // RGB channels
		GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR), // Alpha
		GPU_TEVOPERANDS(0, 0, 0), // RGB
		GPU_TEVOPERANDS(0, 0, 0), // Alpha
		GPU_MODULATE, GPU_MODULATE, // RGB, Alpha
		0xFFFFFFFF);

	// Configure the first texture unit
	GPU_SetTextureEnable(GPU_TEXUNIT0);
	GPU_SetTexture(
		GPU_TEXUNIT0,
		(u32*)osConvertVirtToPhys((u32)tex_data),
		64, // Width
		64, // Height
		GPU_TEXTURE_MAG_FILTER(GPU_LINEAR) | GPU_TEXTURE_WRAP_S(GPU_REPEAT) | GPU_TEXTURE_WRAP_T(GPU_REPEAT), // Flags
		GPU_RGBA8 // Pixel format
	);

	// Configure the "attribute buffers" (that is, the vertex input buffers)
	GPU_SetAttributeBuffers(
		3, // Number of inputs per vertex
		(u32*)osConvertVirtToPhys((u32)vbo_data), // Location of the VBO
		GPU_ATTRIBFMT(0, 3, GPU_FLOAT) | // Format of the inputs
		GPU_ATTRIBFMT(1, 2, GPU_FLOAT) |
		GPU_ATTRIBFMT(2, 3, GPU_FLOAT),
		0xFFC, // Unused attribute mask, in our case bits 0~2 are cleared since they are used
		0x210, // Attribute permutations (here it is the identity, passing each attribute in order)
		1, // Number of buffers
		(u32[]) { 0x0 }, // Buffer offsets (placeholders)
		(u64[]) { 0x210 }, // Attribute permutations for each buffer (identity again)
		(u8[])  { 3 }); // Number of attributes for each buffer

	// Calculate the modelView matrix
	matrix_4x4 modelView;
	m4x4_identity(&modelView);
	m4x4_translate(&modelView, 0.0, 0.0, -2.0 + 0.5*sinf(angleX));
	m4x4_rotate_x(&modelView, angleX, true);
	m4x4_rotate_y(&modelView, angleY, true);

	// Rotate the cube each frame
	angleX += M_PI / 180;
	angleY += M_PI / 360;

	// Upload the uniforms
	GPU_SetFloatUniformMatrix(GPU_VERTEX_SHADER, uLoc_projection, &projection);
	GPU_SetFloatUniformMatrix(GPU_VERTEX_SHADER, uLoc_modelView,  &modelView);
	GPU_SetFloatUniformMatrix(GPU_VERTEX_SHADER, uLoc_material,   &material);
	GPU_SetFloatUniform(GPU_VERTEX_SHADER, uLoc_lightVec,     (u32*)(float[]){0.0f, -1.0f, 0.0f, 0.0f}, 1);
	GPU_SetFloatUniform(GPU_VERTEX_SHADER, uLoc_lightHalfVec, (u32*)(float[]){0.0f, -1.0f, 0.0f, 0.0f}, 1);
	GPU_SetFloatUniform(GPU_VERTEX_SHADER, uLoc_lightClr,     (u32*)(float[]){1.0f,  1.0f, 1.0f, 1.0f}, 1);

	// Draw the VBO
	GPU_DrawArray(GPU_TRIANGLES, vertex_list_count);
}

static void sceneExit(void)
{
	// Free the texture
	linearFree(tex_data);

	// Free the VBO
	linearFree(vbo_data);

	// Free the shader program
	shaderProgramFree(&program);
	DVLB_Free(vshader_dvlb);
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
