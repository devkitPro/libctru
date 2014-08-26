#ifndef MATH_H

void loadIdentity44(float* m);
void multMatrix44(float* m1, float* m2, float* m);

void translateMatrix(float* tm, float x, float y, float z);
void rotateMatrixX(float* tm, float x);
void rotateMatrixZ(float* tm, float x);
void scaleMatrix(float* tm, float x, float y, float z);

void initProjectionMatrix(float* m, float fovy, float aspect, float near, float far);

#endif
