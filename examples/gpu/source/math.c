#include <math.h>
#include <string.h>

#include "math.h"

void loadIdentity44(float* m)
{
	if(!m)return;

	memset(m, 0x00, 16*4);
	m[0]=m[5]=m[10]=m[15]=1.0f;
}

void multMatrix44(float* m1, float* m2, float* m) //4x4
{
	int i, j;
	for(i=0;i<4;i++)for(j=0;j<4;j++)m[i+j*4]=(m1[0+j*4]*m2[i+0*4])+(m1[1+j*4]*m2[i+1*4])+(m1[2+j*4]*m2[i+2*4])+(m1[3+j*4]*m2[i+3*4]);
}

void translateMatrix(float* tm, float x, float y, float z)
{
	float rm[16], m[16];

	loadIdentity44(rm);
	rm[3]=x;
	rm[7]=y;
	rm[11]=z;
	
	multMatrix44(rm,tm,m);
	memcpy(tm,m,16*sizeof(float));
}

void rotateMatrixX(float* tm, float x)
{
	float rm[16], m[16];
	memset(rm, 0x00, 16*4);
	rm[0]=1.0f;
	rm[5]=cos(x);
	rm[6]=sin(x);
	rm[9]=-sin(x);
	rm[10]=cos(x);
	rm[15]=1.0f;
	multMatrix44(tm,rm,m);
	memcpy(tm,m,16*sizeof(float));
}

void rotateMatrixZ(float* tm, float x)
{
	float rm[16], m[16];
	memset(rm, 0x00, 16*4);
	rm[0]=cos(x);
	rm[1]=sin(x);
	rm[4]=-sin(x);
	rm[5]=cos(x);
	rm[10]=1.0f;
	rm[15]=1.0f;
	multMatrix44(tm,rm,m);
	memcpy(tm,m,16*sizeof(float));
}

void scaleMatrix(float* tm, float x, float y, float z)
{
	tm[0]*=x; tm[4]*=x; tm[8]*=x; tm[12]*=x;
	tm[1]*=y; tm[5]*=y; tm[9]*=y; tm[13]*=y;
	tm[2]*=z; tm[6]*=z; tm[10]*=z; tm[14]*=z;
}

void initProjectionMatrix(float* m, float fovy, float aspect, float near, float far)
{
	float top = near*tan(fovy/2);
	float right = (top*aspect);
	
	*(m++) = near/right;
	*(m++) = 0.0f;
	*(m++) = 0.0f;
	*(m++) = 0.0f;

	*(m++) = 0.0f;
	*(m++) = near/top;
	*(m++) = 0.0f;
	*(m++) = 0.0f;

	*(m++) = 0.0f;
	*(m++) = 0.0f;
	// *(m++) = -(far+near)/(far-near);
	*(m++) = 0.0f;
	// *(m++) = -2.0f*(far*near)/(far-near);
	// *(m++) = 1.0f;
	*(m++) = -1.0f;

	*(m++) = 0.0f;
	*(m++) = 0.0f;
	*(m++) = -1.0f;
	*(m++) = 0.0f;
}
