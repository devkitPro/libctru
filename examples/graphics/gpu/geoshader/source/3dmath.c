#include "3dmath.h"

void m4x4_identity(matrix_4x4* out)
{
	m4x4_zeros(out);
	out->r[0].x = out->r[1].y = out->r[2].z = out->r[3].w = 1.0f;
}

void m4x4_multiply(matrix_4x4* out, const matrix_4x4* a, const matrix_4x4* b)
{
	int i, j;
	for (i = 0; i < 4; i ++)
		for (j = 0; j < 4; j ++)
			out->r[j].c[i] = a->r[j].x*b->r[0].c[i] + a->r[j].y*b->r[1].c[i] + a->r[j].z*b->r[2].c[i] + a->r[j].w*b->r[3].c[i];
}

void m4x4_translate(matrix_4x4* mtx, float x, float y, float z)
{
	matrix_4x4 tm, om;

	m4x4_identity(&tm);
	tm.r[0].w = x;
	tm.r[1].w = y;
	tm.r[2].w = z;

	m4x4_multiply(&om, mtx, &tm);
	m4x4_copy(mtx, &om);
}

void m4x4_scale(matrix_4x4* mtx, float x, float y, float z)
{
	int i;
	for (i = 0; i < 4; i ++)
	{
		mtx->r[i].x *= x;
		mtx->r[i].y *= y;
		mtx->r[i].z *= z;
	}
}

void m4x4_rotate_x(matrix_4x4* mtx, float angle, bool bRightSide)
{
	matrix_4x4 rm, om;

	float cosAngle = cosf(angle);
	float sinAngle = sinf(angle);

	m4x4_zeros(&rm);
	rm.r[0].x = 1.0f;
	rm.r[1].y = cosAngle;
	rm.r[1].z = sinAngle;
	rm.r[2].y = -sinAngle;
	rm.r[2].z = cosAngle;
	rm.r[3].w = 1.0f;

	if (bRightSide) m4x4_multiply(&om, mtx, &rm);
	else            m4x4_multiply(&om, &rm, mtx);
	m4x4_copy(mtx, &om);
}

void m4x4_rotate_y(matrix_4x4* mtx, float angle, bool bRightSide)
{
	matrix_4x4 rm, om;

	float cosAngle = cosf(angle);
	float sinAngle = sinf(angle);

	m4x4_zeros(&rm);
	rm.r[0].x = cosAngle;
	rm.r[0].z = sinAngle;
	rm.r[1].y = 1.0f;
	rm.r[2].x = -sinAngle;
	rm.r[2].z = cosAngle;
	rm.r[3].w = 1.0f;

	if (bRightSide) m4x4_multiply(&om, mtx, &rm);
	else            m4x4_multiply(&om, &rm, mtx);
	m4x4_copy(mtx, &om);
}

void m4x4_rotate_z(matrix_4x4* mtx, float angle, bool bRightSide)
{
	matrix_4x4 rm, om;

	float cosAngle = cosf(angle);
	float sinAngle = sinf(angle);

	m4x4_zeros(&rm);
	rm.r[0].x = cosAngle;
	rm.r[0].y = sinAngle;
	rm.r[1].x = -sinAngle;
	rm.r[1].y = cosAngle;
	rm.r[2].z = 1.0f;
	rm.r[3].w = 1.0f;

	if (bRightSide) m4x4_multiply(&om, mtx, &rm);
	else            m4x4_multiply(&om, &rm, mtx);
	m4x4_copy(mtx, &om);
}

void m4x4_ortho_tilt(matrix_4x4* mtx, float left, float right, float bottom, float top, float near, float far)
{
	matrix_4x4 mp;
	m4x4_zeros(&mp);

	// Build standard orthogonal projection matrix
	mp.r[0].x = 2.0f / (right - left);
	mp.r[0].w = (left + right) / (left - right);
	mp.r[1].y = 2.0f / (top - bottom);
	mp.r[1].w = (bottom + top) / (bottom - top);
	mp.r[2].z = 2.0f / (near - far);
	mp.r[2].w = (far + near) / (far - near);
	mp.r[3].w = 1.0f;

	// Fix depth range to [-1, 0]
	matrix_4x4 mp2, mp3;
	m4x4_identity(&mp2);
	mp2.r[2].z = 0.5;
	mp2.r[2].w = -0.5;
	m4x4_multiply(&mp3, &mp2, &mp);

	// Fix the 3DS screens' orientation by swapping the X and Y axis
	m4x4_identity(&mp2);
	mp2.r[0].x = 0.0;
	mp2.r[0].y = 1.0;
	mp2.r[1].x = -1.0; // flipped
	mp2.r[1].y = 0.0;
	m4x4_multiply(mtx, &mp2, &mp3);
}

void m4x4_persp_tilt(matrix_4x4* mtx, float fovx, float invaspect, float near, float far)
{
	// Notes:
	// We are passed "fovy" and the "aspect ratio". However, the 3DS screens are sideways,
	// and so are these parameters -- in fact, they are actually the fovx and the inverse
	// of the aspect ratio. Therefore the formula for the perspective projection matrix
	// had to be modified to be expressed in these terms instead.

	// Notes:
	// fovx = 2 atan(tan(fovy/2)*w/h)
	// fovy = 2 atan(tan(fovx/2)*h/w)
	// invaspect = h/w

	// a0,0 = h / (w*tan(fovy/2)) =
	//      = h / (w*tan(2 atan(tan(fovx/2)*h/w) / 2)) =
	//      = h / (w*tan( atan(tan(fovx/2)*h/w) )) =
	//      = h / (w * tan(fovx/2)*h/w) =
	//      = 1 / tan(fovx/2)

	// a1,1 = 1 / tan(fovy/2) = (...) = w / (h*tan(fovx/2))

	float fovx_tan = tanf(fovx / 2);
	matrix_4x4 mp;
	m4x4_zeros(&mp);

	// Build standard perspective projection matrix
	mp.r[0].x = 1.0f / fovx_tan;
	mp.r[1].y = 1.0f / (fovx_tan*invaspect);
	mp.r[2].z = (near + far) / (near - far);
	mp.r[2].w = (2 * near * far) / (near - far);
	mp.r[3].z = -1.0f;

	// Fix depth range to [-1, 0]
	matrix_4x4 mp2;
	m4x4_identity(&mp2);
	mp2.r[2].z = 0.5;
	mp2.r[2].w = -0.5;
	m4x4_multiply(mtx, &mp2, &mp);

	// Rotate the matrix one quarter of a turn CCW in order to fix the 3DS screens' orientation
	m4x4_rotate_z(mtx, M_PI / 2, true);
}
