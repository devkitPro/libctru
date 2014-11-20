#ifndef MATH_H
#define MATH_H

#include <3ds/types.h>
#include <math.h>

typedef float mtx44[4][4];
typedef float mtx33[3][3];

typedef struct
{
	s32 x, y, z;
}vect3Di_s;

static inline vect3Di_s vect3Di(s32 x, s32 y, s32 z)
{
	return (vect3Di_s){x,y,z};
}

static inline vect3Di_s vaddi(vect3Di_s u, vect3Di_s v)
{
	return (vect3Di_s){u.x+v.x,u.y+v.y,u.z+v.z};
}

static inline vect3Di_s vsubi(vect3Di_s u, vect3Di_s v)
{
	return (vect3Di_s){u.x-v.x,u.y-v.y,u.z-v.z};
}

static inline vect3Di_s vmuli(vect3Di_s v, s32 f)
{
	return (vect3Di_s){v.x*f,v.y*f,v.z*f};
}

typedef struct
{
	float x, y, z;
}vect3Df_s;

static inline vect3Df_s vect3Df(float x, float y, float z)
{
	return (vect3Df_s){x,y,z};
}

static inline vect3Df_s vaddf(vect3Df_s u, vect3Df_s v)
{
	return (vect3Df_s){u.x+v.x,u.y+v.y,u.z+v.z};
}

static inline vect3Df_s vsubf(vect3Df_s u, vect3Df_s v)
{
	return (vect3Df_s){u.x-v.x,u.y-v.y,u.z-v.z};
}

static inline vect3Df_s vmulf(vect3Df_s v, float f)
{
	return (vect3Df_s){v.x*f,v.y*f,v.z*f};
}

static inline vect3Df_s vscalef(vect3Df_s v1, vect3Df_s v2)
{
	return (vect3Df_s){v1.x*v2.x,v1.y*v2.y,v1.z*v2.z};
}

static inline float vmagf(vect3Df_s v)
{
	return sqrtf(v.x*v.x+v.y*v.y+v.z*v.z);
}

static inline float vdistf(vect3Df_s v1, vect3Df_s v2)
{
	return sqrtf((v1.x-v2.x)*(v1.x-v2.x)+(v1.y-v2.y)*(v1.y-v2.y)+(v1.z-v2.z)*(v1.z-v2.z));
}

static inline vect3Df_s vnormf(vect3Df_s v)
{
	const float l=sqrtf(v.x*v.x+v.y*v.y+v.z*v.z);
	return (vect3Df_s){v.x/l,v.y/l,v.z/l};
}

typedef struct
{
	float x, y, z, w;
}vect4Df_s;

static inline vect4Df_s vect4Df(float x, float y, float z, float w)
{
	return (vect4Df_s){x,y,z,w};
}

static inline vect4Df_s vaddf4(vect4Df_s u, vect4Df_s v)
{
	return (vect4Df_s){u.x+v.x,u.y+v.y,u.z+v.z,u.w+v.w};
}

static inline vect4Df_s vsubf4(vect4Df_s u, vect4Df_s v)
{
	return (vect4Df_s){u.x-v.x,u.y-v.y,u.z-v.z,u.w-v.w};
}

static inline vect4Df_s vmulf4(vect4Df_s v, float f)
{
	return (vect4Df_s){v.x*f,v.y*f,v.z*f,v.w*f};
}

static inline float vdotf4(vect4Df_s v1, vect4Df_s v2)
{
	return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z+v1.w*v2.w;
}

static inline vect4Df_s vnormf4(vect4Df_s v)
{
	const float l=sqrtf(v.x*v.x+v.y*v.y+v.z*v.z+v.w*v.w);
	return (vect4Df_s){v.x/l,v.y/l,v.z/l,v.w/l};
}

//interstuff
static inline vect3Di_s vf2i(vect3Df_s v)
{
	return (vect3Di_s){floorf(v.x),floorf(v.y),floorf(v.z)};
}

static inline vect3Df_s vi2f(vect3Di_s v)
{
	return (vect3Df_s){(float)v.x,(float)v.y,(float)v.z};
}

void loadIdentity44(float* m);
void multMatrix44(float* m1, float* m2, float* m);

void translateMatrix(float* tm, float x, float y, float z);
void rotateMatrixX(float* tm, float x, bool r);
void rotateMatrixY(float* tm, float x, bool r);
void rotateMatrixZ(float* tm, float x, bool r);
void scaleMatrix(float* tm, float x, float y, float z);

void initProjectionMatrix(float* m, float fovy, float aspect, float near, float far);

vect3Df_s getMatrixColumn(float* m, u8 i);
vect3Df_s getMatrixRow(float* m, u8 i);
vect4Df_s getMatrixColumn4(float* m, u8 i);
vect4Df_s getMatrixRow4(float* m, u8 i);

#endif
