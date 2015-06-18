/**
* Hello Triangle example, made by Lectem
*
* Draws a white triangle using the 3DS GPU.
* This example should give you enough hints and links on how to use the GPU for basic non-textured rendering.
* Another version of this example will be made with colors.
*
* Thanks to smea, fincs, neobrain, xerpi and all those who helped me understand how the 3DS GPU works
*/


#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shader_vsh_shbin.h"
#include "3dutils.h"
#include "mmath.h"



/**
* Crappy assert stuff that lets you go back to hbmenu by pressing start. 
*/
#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
#define my_assert(e) ((e) ? (void)0 : _my_assert("assert failed at " __FILE__ ":" LINE_STRING " (" #e ")\n"))
void _my_assert(char * text)
{
    printf("%s\n",text);
    do{
        hidScanInput();
        if(keysDown()&KEY_START)break;
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }while(aptMainLoop());
    //should stop the program and clean up our mess
}

//falgs used for transfer from GPU output buffer to the actual framebuffer
#define DISPLAY_TRANSFER_FLAGS \
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
     GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
     GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))



typedef struct {
    float x, y, z;
} vector_3f;


typedef struct {
    float r, g, b, a;
} vector_4f;

typedef struct {
    vector_3f position;
    vector_4f color;
} vertex_pos_col;

#define GPU_CMD_SIZE 0x40000

//GPU framebuffer address
u32* gpuFBuffer = NULL;
//GPU depth buffer address
u32* gpuDBuffer = NULL;
//GPU command buffers
u32* gpuCmd = NULL;

//shader structure
DVLB_s* shader_dvlb;    //the header
shaderProgram_s shader; //the program


Result projUniformRegister      =-1;
Result modelviewUniformRegister =-1;

#define ABGR8(r,g,b,a) ((((r)&0xFF)<<24) | (((g)&0xFF)<<16) | (((b)&0xFF)<<8) | (((a)&0xFF)<<0))

//The color used to clear the screen. ctrulib defaults to the ABGR format
u32 clearColor=ABGR8(0x68, 0xB0, 0xD8, 0xFF);

//The projection matrix
static float ortho_matrix[4*4];

void GPU_SetDummyTexEnv(u8 num);
void gpuEndFrame();

void gpuInit()
{

    GPU_Init(NULL);//initialize GPU

    gfxSet3D(false);//We will not be using the 3D mode in this example
    Result res=0;

    /**
    * Load our vertex shader and its uniforms
    * Check http://3dbrew.org/wiki/SHBIN for more informations about the shader binaries
    */
    shader_dvlb = DVLB_ParseFile((u32 *)shader_vsh_shbin, shader_vsh_shbin_size);//load our vertex shader binary
    my_assert(shader_dvlb != NULL);
    shaderProgramInit(&shader);
    res = shaderProgramSetVsh(&shader, &shader_dvlb->DVLE[0]);
    my_assert(res >=0); // check for errors

    //Allocate the GPU render buffers
    gpuFBuffer = vramMemAlign(400*240*8, 0x100);
    gpuDBuffer = vramMemAlign(400*240*8, 0x100);

    //In this example we are only rendering in "2D mode", so we don't need one command buffer per eye
    gpuCmd    = linearAlloc(GPU_CMD_SIZE * (sizeof *gpuCmd) ); //Don't forget that commands size is 4 (hence the sizeof)
    my_assert(gpuCmd != NULL);

    //Reset the gpu
    //This actually needs a command buffer to work, and will then use it as default
    GPU_Reset(NULL, gpuCmd, GPU_CMD_SIZE);

    projUniformRegister = shaderInstanceGetUniformLocation(shader.vertexShader, "projection");
    my_assert(projUniformRegister != -1); // make sure we did get the uniform


    shaderProgramUse(&shader); // Select the shader to use



    initOrthographicMatrix(ortho_matrix, 0.0f, 400.0f, 0.0f, 240.0f, 0.0f, 1.0f); // A basic projection for 2D drawings
    SetUniformMatrix(projUniformRegister, ortho_matrix); // Upload the matrix to the GPU
}

void gpuExit()
{
    //do things properly
    linearFree(gpuCmd);
    vramFree(gpuDBuffer);
    vramFree(gpuFBuffer);
    shaderProgramFree(&shader);
    DVLB_Free(shader_dvlb);
    GPU_Reset(NULL, gpuCmd, GPU_CMD_SIZE); // Not really needed, but safer for the next applications ?
}

void gpuEndFrame()
{
    //Ask the GPU to draw everything (execute the commands)
    GPU_FinishDrawing();
    GPUCMD_Finalize();
    GPUCMD_FlushAndRun(NULL);
    gspWaitForP3D();//Wait for the gpu 3d processing to be done
    //Copy the GPU output buffer to the screen framebuffer
    //See http://3dbrew.org/wiki/GPU#Transfer_Engine for more details about the transfer engine
    GX_SetDisplayTransfer(NULL, // Use ctrulib's gx command buffer
        gpuFBuffer, GX_BUFFER_DIM(240, 400), 
        (u32*)gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), GX_BUFFER_DIM(240, 400), 
        DISPLAY_TRANSFER_FLAGS);
    gspWaitForPPF();

    //Clear the screen
    //See http://3dbrew.org/wiki/GSP_Shared_Memory#Trigger_Memory_Fill for the width0 and width1 arguments (control bits)
    GX_SetMemoryFill(NULL,// Use ctrulib's gx command buffer
        gpuFBuffer, clearColor, &gpuFBuffer[400*240], GX_FILL_TRIGGER | GX_FILL_32BIT_DEPTH, // Fill the framebuffer with clearcolor. 32bit values
        gpuDBuffer, 0x00000000, &gpuDBuffer[400*240], GX_FILL_TRIGGER | GX_FILL_32BIT_DEPTH);// Fill the depthbuffer with clearcolor. 32bit values
    gspWaitForPSC0();
    gfxSwapBuffersGpu();

    //Wait for the screen to be updated
    gspWaitForVBlank();

    //Get ready to start a new frame
    GPUCMD_SetBufferOffset(0);

    //Viewport (http://3dbrew.org/wiki/GPU_Commands#Command_0x0041)
    GPU_SetViewport((u32 *)osConvertVirtToPhys((u32)gpuDBuffer),
            (u32 *)osConvertVirtToPhys((u32)gpuFBuffer),
            0, 0,
            240, 400);//Our screen is 400*240, but remember that the screen is sideways, hence using w=240 and h=400


    GPU_DepthMap(-1.0f, 0.0f);  //Be careful, standard OpenGL clipping is [-1;1], but it is [-1;0] on the pica200
    // Note : this is corrected by our projection matrix !

    //Sets the texture environment parameters not to modify our pixels at fragment stage
    //See https://www.opengl.org/sdk/docs/man2/xhtml/glTexEnv.xml for more insight
    GPU_SetTexEnv(
            0,
            GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
            GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
            GPU_TEVOPERANDS(0,0,0),
            GPU_TEVOPERANDS(0,0,0),
            GPU_REPLACE, GPU_REPLACE,
            0xFFFFFFFF
    );
    GPU_SetDummyTexEnv(1);
    GPU_SetDummyTexEnv(2);
    GPU_SetDummyTexEnv(3);
    GPU_SetDummyTexEnv(4);
    GPU_SetDummyTexEnv(5);
}



//Our data
static const vector_3f triangle_mesh[] =
        {
                {240.0f+60.0f, 120.0f,       0.5f},
                {240.0f-60.0f, 120.0f+60.0f, 0.5f},
                {240.0f-60.0f, 120.0f-60.0f, 0.5f}
        };

static void* triangle_data = NULL;

int main(int argc, char** argv)
{

    srvInit();
    aptInit();
    hidInit(NULL);

    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);


    gpuInit();
    printf("hello triangle !\n");
    triangle_data = linearAlloc(sizeof(triangle_mesh));     //allocate our vbo on the linear heap
    memcpy(triangle_data, triangle_mesh, sizeof(triangle_mesh)); //Copy our data

    do{
        hidScanInput();
        if(keysDown()&KEY_START)break; //Stop the program when Start is pressed

        //Setup the buffers data
        GPU_SetAttributeBuffers(
                1, // number of attributes
                (u32 *) osConvertVirtToPhys((u32) triangle_data),
                GPU_ATTRIBFMT(0, 3, GPU_FLOAT),//We only have vertices
                0xFFFE,//Attribute mask, in our case 0b1110 since we use only the first one
                0x0,//Attribute permutations (here it is the identity)
                1, //number of buffers
                (u32[]) {0x0}, // buffer offsets (placeholders)
                (u64[]) {0x0}, // attribute permutations for each buffer (identity again)
                (u8[]) {1} // number of attributes for each buffer
        );
        //Display the buffers data
        GPU_DrawArray(GPU_TRIANGLES, sizeof(triangle_mesh) / sizeof(triangle_mesh[0]));

        gpuEndFrame();

    }while(aptMainLoop());
    gpuExit();


    gfxExit();
    hidExit();
    aptExit();
    srvExit();

    return 0;
}


void GPU_SetDummyTexEnv(u8 num)
{
    //Don't touch the colors of the previous stages
    GPU_SetTexEnv(num,
            GPU_TEVSOURCES(GPU_PREVIOUS, 0, 0),
            GPU_TEVSOURCES(GPU_PREVIOUS, 0, 0),
            GPU_TEVOPERANDS(0,0,0),
            GPU_TEVOPERANDS(0,0,0),
            GPU_REPLACE,
            GPU_REPLACE,
            0xFFFFFFFF);
}