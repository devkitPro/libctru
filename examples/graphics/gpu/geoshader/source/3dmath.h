/*
 * Bare-bones simplistic 3D math library
 * This library is common to all libctru GPU examples
 */

#pragma once
#include <string.h>
#include <stdbool.h>
#include <math.h>

typedef union { struct { float w, z, y, x; }; float c[4]; } vector_4f;
typedef struct { vector_4f r[4]; } matrix_4x4;

static inline float v4f_dp4(const vector_4f* a, const vector_4f* b)
{
	return a->x*b->x + a->y*b->y + a->z*b->z + a->w*b->w;
}

static inline float v4f_mod4(const vector_4f* a)
{
	return sqrtf(v4f_dp4(a,a));
}

static inline void v4f_norm4(vector_4f* vec)
{
	float m = v4f_mod4(vec);
	if (m == 0.0) return;
	vec->x /= m;
	vec->y /= m;
	vec->z /= m;
	vec->w /= m;
}

static inline void m4x4_zeros(matrix_4x4* out)
{
	memset(out, 0, sizeof(*out));
}

static inline void m4x4_copy(matrix_4x4* out, const matrix_4x4* in)
{
	memcpy(out, in, sizeof(*out));
}

void m4x4_identity(matrix_4x4* out);
void m4x4_multiply(matrix_4x4* out, const matrix_4x4* a, const matrix_4x4* b);

void m4x4_translate(matrix_4x4* mtx, float x, float y, float z);
void m4x4_scale(matrix_4x4* mtx, float x, float y, float z);

void m4x4_rotate_x(matrix_4x4* mtx, float angle, bool bRightSide);
void m4x4_rotate_y(matrix_4x4* mtx, float angle, bool bRightSide);
void m4x4_rotate_z(matrix_4x4* mtx, float angle, bool bRightSide);

// Special versions of the projection matrices that take the 3DS' screen orientation into account
void m4x4_ortho_tilt(matrix_4x4* mtx, float left, float right, float bottom, float top, float near, float far);
void m4x4_persp_tilt(matrix_4x4* mtx, float fovy, float aspect, float near, float far);
