//
// Created by Lectem on 22/03/2015.
//

#include "3dutils.h"

void SetUniformMatrix(u32 startreg, float* m) {
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
