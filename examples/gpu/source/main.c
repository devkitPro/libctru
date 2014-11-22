///////////////////////////////////////
//            GPU example            //
///////////////////////////////////////

//this example is meant to show how to use the GPU to render a 3D object
//it also shows how to do stereoscopic 3D
//it uses GS which is a WIP GPU abstraction layer that's currently part of 3DScraft
//keep in mind GPU reverse engineering is an ongoing effort and our understanding of it is still fairly limited.

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <3ds.h>

#include "math.h"
#include "gs.h"

#include "test_vsh_shbin.h"
#include "texture_bin.h"

//will be moved into ctrulib at some point
#define CONFIG_3D_SLIDERSTATE (*(float*)0x1FF81080)

#define RGBA8(r,g,b,a) ((((r)&0xFF)<<24) | (((g)&0xFF)<<16) | (((b)&0xFF)<<8) | (((a)&0xFF)<<0))

//shader structure
DVLB_s* shader;
//texture data pointer
u32* texData;
//vbo structure
gsVbo_s vbo;

//GPU framebuffer address
u32* gpuOut=(u32*)0x1F119400;
//GPU depth buffer address
u32* gpuDOut=(u32*)0x1F370800;

//angle for the vertex lighting (cf test.vsh)
float lightAngle;
//object position and rotation angle
vect3Df_s position, angle;

//vertex structure
typedef struct
{
	vect3Df_s position;
	float texcoord[2];
	vect3Df_s normal;
}vertex_s;

//object data (cube)
//obviously this doesn't have to be defined manually, but we will here for the purposes of the example
//each line is a vertex : {position.x, position.y, position.z}, {texcoord.t, texcoord.s}, {normal.x, normal.y, normal.z}
//we're drawing triangles so three lines = one triangle
const vertex_s modelVboData[]=
{
	//first face (PZ)
		//first triangle
		{(vect3Df_s){-0.5f, -0.5f, +0.5f}, (float[]){0.0f, 1.0f}, (vect3Df_s){0.0f, 0.0f, +1.0f}},
		{(vect3Df_s){+0.5f, -0.5f, +0.5f}, (float[]){1.0f, 1.0f}, (vect3Df_s){0.0f, 0.0f, +1.0f}},
		{(vect3Df_s){+0.5f, +0.5f, +0.5f}, (float[]){1.0f, 0.0f}, (vect3Df_s){0.0f, 0.0f, +1.0f}},
		//second triangle
		{(vect3Df_s){+0.5f, +0.5f, +0.5f}, (float[]){1.0f, 0.0f}, (vect3Df_s){0.0f, 0.0f, +1.0f}},
		{(vect3Df_s){-0.5f, +0.5f, +0.5f}, (float[]){0.0f, 0.0f}, (vect3Df_s){0.0f, 0.0f, +1.0f}},
		{(vect3Df_s){-0.5f, -0.5f, +0.5f}, (float[]){0.0f, 1.0f}, (vect3Df_s){0.0f, 0.0f, +1.0f}},
	//second face (MZ)
		//first triangle
		{(vect3Df_s){-0.5f, -0.5f, -0.5f}, (float[]){0.0f, 1.0f}, (vect3Df_s){0.0f, 0.0f, -1.0f}},
		{(vect3Df_s){-0.5f, +0.5f, -0.5f}, (float[]){1.0f, 1.0f}, (vect3Df_s){0.0f, 0.0f, -1.0f}},
		{(vect3Df_s){+0.5f, +0.5f, -0.5f}, (float[]){1.0f, 0.0f}, (vect3Df_s){0.0f, 0.0f, -1.0f}},
		//second triangle
		{(vect3Df_s){+0.5f, +0.5f, -0.5f}, (float[]){1.0f, 0.0f}, (vect3Df_s){0.0f, 0.0f, -1.0f}},
		{(vect3Df_s){+0.5f, -0.5f, -0.5f}, (float[]){0.0f, 0.0f}, (vect3Df_s){0.0f, 0.0f, -1.0f}},
		{(vect3Df_s){-0.5f, -0.5f, -0.5f}, (float[]){0.0f, 1.0f}, (vect3Df_s){0.0f, 0.0f, -1.0f}},
	//third face (PX)
		//first triangle
		{(vect3Df_s){+0.5f, -0.5f, -0.5f}, (float[]){0.0f, 1.0f}, (vect3Df_s){+1.0f, 0.0f, 0.0f}},
		{(vect3Df_s){+0.5f, +0.5f, -0.5f}, (float[]){1.0f, 1.0f}, (vect3Df_s){+1.0f, 0.0f, 0.0f}},
		{(vect3Df_s){+0.5f, +0.5f, +0.5f}, (float[]){1.0f, 0.0f}, (vect3Df_s){+1.0f, 0.0f, 0.0f}},
		//second triangle
		{(vect3Df_s){+0.5f, +0.5f, +0.5f}, (float[]){1.0f, 0.0f}, (vect3Df_s){+1.0f, 0.0f, 0.0f}},
		{(vect3Df_s){+0.5f, -0.5f, +0.5f}, (float[]){0.0f, 0.0f}, (vect3Df_s){+1.0f, 0.0f, 0.0f}},
		{(vect3Df_s){+0.5f, -0.5f, -0.5f}, (float[]){0.0f, 1.0f}, (vect3Df_s){+1.0f, 0.0f, 0.0f}},
	//fourth face (MX)
		//first triangle
		{(vect3Df_s){-0.5f, -0.5f, -0.5f}, (float[]){0.0f, 1.0f}, (vect3Df_s){-1.0f, 0.0f, 0.0f}},
		{(vect3Df_s){-0.5f, -0.5f, +0.5f}, (float[]){1.0f, 1.0f}, (vect3Df_s){-1.0f, 0.0f, 0.0f}},
		{(vect3Df_s){-0.5f, +0.5f, +0.5f}, (float[]){1.0f, 0.0f}, (vect3Df_s){-1.0f, 0.0f, 0.0f}},
		//second triangle
		{(vect3Df_s){-0.5f, +0.5f, +0.5f}, (float[]){1.0f, 0.0f}, (vect3Df_s){-1.0f, 0.0f, 0.0f}},
		{(vect3Df_s){-0.5f, +0.5f, -0.5f}, (float[]){0.0f, 0.0f}, (vect3Df_s){-1.0f, 0.0f, 0.0f}},
		{(vect3Df_s){-0.5f, -0.5f, -0.5f}, (float[]){0.0f, 1.0f}, (vect3Df_s){-1.0f, 0.0f, 0.0f}},
	//fifth face (PY)
		//first triangle
		{(vect3Df_s){-0.5f, +0.5f, -0.5f}, (float[]){0.0f, 1.0f}, (vect3Df_s){0.0f, +1.0f, 0.0f}},
		{(vect3Df_s){-0.5f, +0.5f, +0.5f}, (float[]){1.0f, 1.0f}, (vect3Df_s){0.0f, +1.0f, 0.0f}},
		{(vect3Df_s){+0.5f, +0.5f, +0.5f}, (float[]){1.0f, 0.0f}, (vect3Df_s){0.0f, +1.0f, 0.0f}},
		//second triangle
		{(vect3Df_s){+0.5f, +0.5f, +0.5f}, (float[]){1.0f, 0.0f}, (vect3Df_s){0.0f, +1.0f, 0.0f}},
		{(vect3Df_s){+0.5f, +0.5f, -0.5f}, (float[]){0.0f, 0.0f}, (vect3Df_s){0.0f, +1.0f, 0.0f}},
		{(vect3Df_s){-0.5f, +0.5f, -0.5f}, (float[]){0.0f, 1.0f}, (vect3Df_s){0.0f, +1.0f, 0.0f}},
	//sixth face (MY)
		//first triangle
		{(vect3Df_s){-0.5f, -0.5f, -0.5f}, (float[]){0.0f, 1.0f}, (vect3Df_s){0.0f, -1.0f, 0.0f}},
		{(vect3Df_s){+0.5f, -0.5f, -0.5f}, (float[]){1.0f, 1.0f}, (vect3Df_s){0.0f, -1.0f, 0.0f}},
		{(vect3Df_s){+0.5f, -0.5f, +0.5f}, (float[]){1.0f, 0.0f}, (vect3Df_s){0.0f, -1.0f, 0.0f}},
		//second triangle
		{(vect3Df_s){+0.5f, -0.5f, +0.5f}, (float[]){1.0f, 0.0f}, (vect3Df_s){0.0f, -1.0f, 0.0f}},
		{(vect3Df_s){-0.5f, -0.5f, +0.5f}, (float[]){0.0f, 0.0f}, (vect3Df_s){0.0f, -1.0f, 0.0f}},
		{(vect3Df_s){-0.5f, -0.5f, -0.5f}, (float[]){0.0f, 1.0f}, (vect3Df_s){0.0f, -1.0f, 0.0f}},
};

//stolen from staplebutt
void GPU_SetDummyTexEnv(u8 num)
{
	GPU_SetTexEnv(num, 
		GPU_TEVSOURCES(GPU_PREVIOUS, 0, 0), 
		GPU_TEVSOURCES(GPU_PREVIOUS, 0, 0), 
		GPU_TEVOPERANDS(0,0,0), 
		GPU_TEVOPERANDS(0,0,0), 
		GPU_REPLACE, 
		GPU_REPLACE, 
		0xFFFFFFFF);
}

// topscreen
void renderFrame()
{
	GPU_SetViewport((u32*)osConvertVirtToPhys((u32)gpuDOut),(u32*)osConvertVirtToPhys((u32)gpuOut),0,0,240*2,400);
	
	GPU_DepthRange(-1.0f, 0.0f);
	GPU_SetFaceCulling(GPU_CULL_BACK_CCW);
	GPU_SetStencilTest(false, GPU_ALWAYS, 0x00, 0xFF, 0x00);
	GPU_SetStencilOp(GPU_KEEP, GPU_KEEP, GPU_KEEP);
	GPU_SetBlendingColor(0,0,0,0);
	GPU_SetDepthTestAndWriteMask(true, GPU_GREATER, GPU_WRITE_ALL);
	
	GPUCMD_AddSingleParam(0x00010062, 0); 
	GPUCMD_AddSingleParam(0x000F0118, 0);
	
	//setup shader
	SHDR_UseProgram(shader, 0);
	
	GPU_SetAlphaBlending(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
	GPU_SetAlphaTest(false, GPU_ALWAYS, 0x00);
	
	GPU_SetTextureEnable(GPU_TEXUNIT0);
	
	GPU_SetTexEnv(0, 
		GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR), 
		GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
		GPU_TEVOPERANDS(0,0,0), 
		GPU_TEVOPERANDS(0,0,0), 
		GPU_MODULATE, GPU_MODULATE, 
		0xFFFFFFFF);
	GPU_SetDummyTexEnv(1);
	GPU_SetDummyTexEnv(2);
	GPU_SetDummyTexEnv(3);
	GPU_SetDummyTexEnv(4);
	GPU_SetDummyTexEnv(5);

	//texturing stuff
		GPU_SetTexture(GPU_TEXUNIT0, (u32*)osConvertVirtToPhys((u32)texData),128,128,GPU_TEXTURE_MAG_FILTER(GPU_NEAREST)|GPU_TEXTURE_MIN_FILTER(GPU_NEAREST),GPU_RGBA8);
		GPU_SetAttributeBuffers(3, (u32*)osConvertVirtToPhys((u32)texData),
			GPU_ATTRIBFMT(0, 3, GPU_FLOAT)|GPU_ATTRIBFMT(1, 2, GPU_FLOAT)|GPU_ATTRIBFMT(2, 3, GPU_FLOAT),
			0xFFC, 0x210, 1, (u32[]){0x00000000}, (u64[]){0x210}, (u8[]){3});

	//setup lighting (this is specific to our shader)
		vect3Df_s lightDir=vnormf(vect3Df(cos(lightAngle), -1.0f, sin(lightAngle)));
		GPU_SetUniform(SHDR_GetUniformRegister(shader, "lightDirection", 0), (u32*)(float[]){0.0f, -lightDir.z, -lightDir.y, -lightDir.x}, 1);
		GPU_SetUniform(SHDR_GetUniformRegister(shader, "lightAmbient", 0), (u32*)(float[]){0.7f, 0.4f, 0.4f, 0.4f}, 1);

	//initialize projection matrix to standard perspective stuff
	gsMatrixMode(GS_PROJECTION);
	gsProjectionMatrix(80.0f*M_PI/180.0f, 240.0f/400.0f, 0.01f, 100.0f);
	gsRotateZ(M_PI/2); //because framebuffer is sideways...

	//draw object
		gsMatrixMode(GS_MODELVIEW);
		gsPushMatrix();
			gsTranslate(position.x, position.y, position.z);
			gsRotateX(angle.x);
			gsRotateY(angle.y);
			gsVboDraw(&vbo);
		gsPopMatrix();
	GPU_FinishDrawing();
}

int main(int argc, char** argv)
{
	//setup services
	srvInit();	
	aptInit();
	gfxInit();
	hidInit(NULL);

	//initialize GPU
	GPU_Init(NULL);

	//let GFX know we're ok with doing stereoscopic 3D rendering
	gfxSet3D(true);

	//load our vertex shader binary
	shader=SHDR_ParseSHBIN((u32*)test_vsh_shbin, test_vsh_shbin_size);

	//initialize GS
	gsInit(shader);

	//allocate our GPU command buffers
	//they *have* to be on the linear heap
	u32 gpuCmdSize=0x40000;
	u32* gpuCmd=(u32*)linearAlloc(gpuCmdSize*4);
	u32* gpuCmdRight=(u32*)linearAlloc(gpuCmdSize*4);

	//actually reset the GPU
	GPU_Reset(NULL, gpuCmd, gpuCmdSize);

	//create texture
	texData=(u32*)linearMemAlign(texture_bin_size, 0x80); //textures need to be 0x80-byte aligned
	memcpy(texData, texture_bin, texture_bin_size);

	//create VBO
	gsVboInit(&vbo);
	gsVboCreate(&vbo, sizeof(modelVboData));
	gsVboAddData(&vbo, (void*)modelVboData, sizeof(modelVboData), sizeof(modelVboData)/sizeof(vertex_s));
	gsVboFlushData(&vbo);

	//initialize object position and angle
	position=vect3Df(0.0f, 0.0f, -2.0f);
	angle=vect3Df(M_PI/4, M_PI/4, 0.0f);

	//background color (blue)
	u32 backgroundColor=RGBA8(0x68, 0xB0, 0xD8, 0xFF);

	while(aptMainLoop())
	{
		//get current 3D slider state
		float slider=CONFIG_3D_SLIDERSTATE;

		//controls
		hidScanInput();
		//START to exit to hbmenu
		if(keysDown()&KEY_START)break;

		//A/B to change vertex lighting angle
		if(keysHeld()&KEY_A)lightAngle+=0.1f;
		if(keysHeld()&KEY_B)lightAngle-=0.1f;

		//D-PAD to rotate object
		if(keysHeld()&KEY_RIGHT)angle.x+=0.05f;
		if(keysHeld()&KEY_LEFT)angle.x-=0.05f;
		if(keysHeld()&KEY_UP)angle.y+=0.05f;
		if(keysHeld()&KEY_DOWN)angle.y-=0.05f;

		//R/L to bring object closer to or move it further from the camera
		if(keysHeld()&KEY_R)position.z+=0.1f;
		if(keysHeld()&KEY_L)position.z-=0.1f;

		//generate our GPU command buffer for this frame
		gsStartFrame();
		renderFrame();
		GPUCMD_Finalize();

		if(slider>0.0f)
		{
			//new and exciting 3D !
			//make a copy of left gpu buffer
			u32 offset; GPUCMD_GetBuffer(NULL, NULL, &offset);
			memcpy(gpuCmdRight, gpuCmd, offset*4);

			//setup interaxial
			float interaxial=slider*0.12f;

			//adjust left gpu buffer fo 3D !
			{mtx44 m; loadIdentity44((float*)m); translateMatrix((float*)m, -interaxial*0.5f, 0.0f, 0.0f); gsAdjustBufferMatrices(m);}

			//draw left framebuffer
			GPUCMD_FlushAndRun(NULL);

			//while GPU starts drawing the left buffer, adjust right one for 3D !
			GPUCMD_SetBuffer(gpuCmdRight, gpuCmdSize, offset);
			{mtx44 m; loadIdentity44((float*)m); translateMatrix((float*)m, interaxial*0.5f, 0.0f, 0.0f); gsAdjustBufferMatrices(m);}

			//we wait for the left buffer to finish drawing
			gspWaitForP3D();
			GX_SetDisplayTransfer(NULL, (u32*)gpuOut, 0x019001E0, (u32*)gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 0x019001E0, 0x01001000);
			gspWaitForPPF();

			//we draw the right buffer, wait for it to finish and then switch back to left one
			//clear the screen
			GX_SetMemoryFill(NULL, (u32*)gpuOut, backgroundColor, (u32*)&gpuOut[0x2EE00], 0x201, (u32*)gpuDOut, 0x00000000, (u32*)&gpuDOut[0x2EE00], 0x201);
			gspWaitForPSC0();

			//draw the right framebuffer
			GPUCMD_FlushAndRun(NULL);
			gspWaitForP3D();

			//transfer from GPU output buffer to actual framebuffer
			GX_SetDisplayTransfer(NULL, (u32*)gpuOut, 0x019001E0, (u32*)gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL), 0x019001E0, 0x01001000);
			gspWaitForPPF();
			GPUCMD_SetBuffer(gpuCmd, gpuCmdSize, 0);
		}else{
			//boring old 2D !

			//draw the frame
			GPUCMD_FlushAndRun(NULL);
			gspWaitForP3D();

			//clear the screen
			GX_SetDisplayTransfer(NULL, (u32*)gpuOut, 0x019001E0, (u32*)gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 0x019001E0, 0x01001000);
			gspWaitForPPF();
		}

		//clear the screen
		GX_SetMemoryFill(NULL, (u32*)gpuOut, backgroundColor, (u32*)&gpuOut[0x2EE00], 0x201, (u32*)gpuDOut, 0x00000000, (u32*)&gpuDOut[0x2EE00], 0x201);
		gspWaitForPSC0();
		gfxSwapBuffersGpu();

		gspWaitForEvent(GSPEVENT_VBlank0, true);
	}

	gsExit();
	hidExit();
	gfxExit();
	aptExit();
	srvExit();
	return 0;
}
