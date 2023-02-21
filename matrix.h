#ifndef MATRIX_H
#define MATRIX_H

#include "vertex.h"

struct mat4 {
	double elems[4][4];
};

typedef struct mat4 mat4;

mat4* matrix_init4 (void* loc, double e11, double e12, double e13, double e14, double e21, double e22, double e23, double e24, double e31, double e32, double e33, double e34, double e41, double e42, double e43, double e44);
mat4* matrix_iden4 (void* loc);
mat4* matrix_translate4 (void* loc, double x, double y, double z);
mat4* matrix_rotx4 (void* loc, double theta);
mat4* matrix_roty4 (void* loc, double theta);
mat4* matrix_rotz4 (void* loc, double theta);
void matrix_mul4m (mat4* res, mat4* a, mat4* b);
void matrix_mul4v (v4* res, mat4* m, v4* v);

#endif