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
	
	multMatrix44(tm,rm,m);
	memcpy(tm,m,16*sizeof(float));
}

// 00 01 02 03
// 04 05 06 07
// 08 09 10 11
// 12 13 14 15

void rotateMatrixX(float* tm, float x, bool r)
{
	float rm[16], m[16];
	memset(rm, 0x00, 16*4);
	rm[0]=1.0f;
	rm[5]=cos(x);
	rm[6]=sin(x);
	rm[9]=-sin(x);
	rm[10]=cos(x);
	rm[15]=1.0f;
	if(!r)multMatrix44(tm,rm,m);
	else multMatrix44(rm,tm,m);
	memcpy(tm,m,16*sizeof(float));
}

void rotateMatrixY(float* tm, float x, bool r)
{
	float rm[16], m[16];
	memset(rm, 0x00, 16*4);
	rm[0]=cos(x);
	rm[2]=sin(x);
	rm[5]=1.0f;
	rm[8]=-sin(x);
	rm[10]=cos(x);
	rm[15]=1.0f;
	if(!r)multMatrix44(tm,rm,m);
	else multMatrix44(rm,tm,m);
	memcpy(tm,m,16*sizeof(float));
}

void rotateMatrixZ(float* tm, float x, bool r)
{
	float rm[16], m[16];
	memset(rm, 0x00, 16*4);
	rm[0]=cos(x);
	rm[1]=sin(x);
	rm[4]=-sin(x);
	rm[5]=cos(x);
	rm[10]=1.0f;
	rm[15]=1.0f;
	if(!r)multMatrix44(tm,rm,m);
	else multMatrix44(rm,tm,m);
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

	float mp[4*4];
	
	mp[0x0] = near/right;
	mp[0x1] = 0.0f;
	mp[0x2] = 0.0f;
	mp[0x3] = 0.0f;

	mp[0x4] = 0.0f;
	mp[0x5] = near/top;
	mp[0x6] = 0.0f;
	mp[0x7] = 0.0f;

	mp[0x8] = 0.0f;
	mp[0x9] = 0.0f;
	mp[0xA] = -(far+near)/(far-near);
	mp[0xB] = -2.0f*(far*near)/(far-near);

	mp[0xC] = 0.0f;
	mp[0xD] = 0.0f;
	mp[0xE] = -1.0f;
	mp[0xF] = 0.0f;

	float mp2[4*4];
	loadIdentity44(mp2);
	mp2[0xA]=0.5;
	mp2[0xB]=-0.5;

	multMatrix44(mp2, mp, m);
}

vect3Df_s getMatrixColumn(float* m, u8 i)
{
	if(!m || i>=4)return vect3Df(0,0,0);
	return vect3Df(m[0+i*4],m[1+i*4],m[2+i*4]);
}

vect3Df_s getMatrixRow(float* m, u8 i)
{
	if(!m || i>=4)return vect3Df(0,0,0);
	return vect3Df(m[i+0*4],m[i+1*4],m[i+2*4]);
}

vect4Df_s getMatrixColumn4(float* m, u8 i)
{
	if(!m || i>=4)return vect4Df(0,0,0,0);
	return vect4Df(m[0+i*4],m[1+i*4],m[2+i*4],m[3+i*4]);
}

vect4Df_s getMatrixRow4(float* m, u8 i)
{
	if(!m || i>=4)return vect4Df(0,0,0,0);
	return vect4Df(m[i+0*4],m[i+1*4],m[i+2*4],m[i+3*4]);
}
