#include "matrix.h"

#include <math.h>

mat4* matrix_init4 (void* loc, double e11, double e12, double e13, double e14, double e21, double e22, double e23, double e24, double e31, double e32, double e33, double e34, double e41, double e42, double e43, double e44) {
	mat4* m = (mat4*)loc;
	m->elems [0][0] = e11;
	m->elems [0][1] = e12;
	m->elems [0][2] = e13;
	m->elems [0][3] = e14;
	m->elems [1][0] = e21;
	m->elems [1][1] = e22;
	m->elems [1][2] = e23;
	m->elems [1][3] = e24;
	m->elems [2][0] = e31;
	m->elems [2][1] = e32;
	m->elems [2][2] = e33;
	m->elems [2][3] = e34;
	m->elems [3][0] = e41;
	m->elems [3][1] = e42;
	m->elems [3][2] = e43;
	m->elems [3][3] = e44;
	return (mat4*)loc;
}

mat4* matrix_iden4 (void* loc) {
	matrix_init4 (loc,	1, 0, 0, 0,
						0, 1, 0, 0,
						0, 0, 1, 0,
						0, 0, 0, 1);
	return (mat4*)loc;
}

mat4* matrix_translate4 (void* loc, double x, double y, double z) {
	matrix_init4 (loc,	1, 0, 0, x,
						0, 1, 0, y,
						0, 0, 1, z,
						0, 0, 0, 1);
	return (mat4*)loc;
}

mat4* matrix_rotx4 (void* loc, double theta) {
	matrix_init4 (loc,	1, 0, 0, 0,
						0, cos (theta), -sin (theta), 0,
						0, sin (theta), cos (theta), 0,
						0, 0, 0, 1);
	return (mat4*)loc;
}

mat4* matrix_roty4 (void* loc, double theta) {
	matrix_init4 (loc,	cos (theta), 0, sin (theta), 0,
						0, 1, 0, 0,
						-sin (theta), 0, cos (theta), 0,
						0, 0, 0, 1);
	return (mat4*)loc;
}

mat4* matrix_rotz4 (void* loc, double theta) {
	matrix_init4 (loc,	cos (theta), -sin (theta), 0, 0,
						sin (theta), cos (theta), 0, 0,
						0, 0, 1, 0,
						0, 0, 0, 1);
	return (mat4*)loc;
}

void matrix_mul4m (mat4* res, mat4* a, mat4* b) {
	res->elems [0][0] = a->elems [0][0] * b->elems [0][0] + a->elems [0][1] * b->elems [1][0] + a->elems [0][2] * b->elems [2][0] + a->elems [0][3] * b->elems [3][0];
	res->elems [0][1] = a->elems [0][0] * b->elems [0][1] + a->elems [0][1] * b->elems [1][1] + a->elems [0][2] * b->elems [2][1] + a->elems [0][3] * b->elems [3][1];
	res->elems [0][2] = a->elems [0][0] * b->elems [0][2] + a->elems [0][1] * b->elems [1][2] + a->elems [0][2] * b->elems [2][2] + a->elems [0][3] * b->elems [3][2];
	res->elems [0][3] = a->elems [0][0] * b->elems [0][3] + a->elems [0][1] * b->elems [1][3] + a->elems [0][2] * b->elems [2][3] + a->elems [0][3] * b->elems [3][3];
	res->elems [1][0] = a->elems [1][0] * b->elems [0][0] + a->elems [1][1] * b->elems [1][0] + a->elems [1][2] * b->elems [2][0] + a->elems [1][3] * b->elems [3][0];
	res->elems [1][1] = a->elems [1][0] * b->elems [0][1] + a->elems [1][1] * b->elems [1][1] + a->elems [1][2] * b->elems [2][1] + a->elems [1][3] * b->elems [3][1];
	res->elems [1][2] = a->elems [1][0] * b->elems [0][2] + a->elems [1][1] * b->elems [1][2] + a->elems [1][2] * b->elems [2][2] + a->elems [1][3] * b->elems [3][2];
	res->elems [1][3] = a->elems [1][0] * b->elems [0][3] + a->elems [1][1] * b->elems [1][3] + a->elems [1][2] * b->elems [2][3] + a->elems [1][3] * b->elems [3][3];
	res->elems [2][0] = a->elems [2][0] * b->elems [0][0] + a->elems [2][1] * b->elems [1][0] + a->elems [2][2] * b->elems [2][0] + a->elems [2][3] * b->elems [3][0];
	res->elems [2][1] = a->elems [2][0] * b->elems [0][1] + a->elems [2][1] * b->elems [1][1] + a->elems [2][2] * b->elems [2][1] + a->elems [2][3] * b->elems [3][1];
	res->elems [2][2] = a->elems [2][0] * b->elems [0][2] + a->elems [2][1] * b->elems [1][2] + a->elems [2][2] * b->elems [2][2] + a->elems [2][3] * b->elems [3][2];
	res->elems [2][3] = a->elems [2][0] * b->elems [0][3] + a->elems [2][1] * b->elems [1][3] + a->elems [2][2] * b->elems [2][3] + a->elems [2][3] * b->elems [3][3];
	res->elems [3][0] = a->elems [0][0] * b->elems [0][0] + a->elems [3][1] * b->elems [1][0] + a->elems [3][2] * b->elems [2][0] + a->elems [3][3] * b->elems [3][0];
	res->elems [3][1] = a->elems [0][0] * b->elems [0][1] + a->elems [3][1] * b->elems [1][1] + a->elems [3][2] * b->elems [2][1] + a->elems [3][3] * b->elems [3][1];
	res->elems [3][2] = a->elems [0][0] * b->elems [0][2] + a->elems [3][1] * b->elems [1][2] + a->elems [3][2] * b->elems [2][2] + a->elems [3][3] * b->elems [3][2];
	res->elems [3][3] = a->elems [0][0] * b->elems [0][3] + a->elems [3][1] * b->elems [1][3] + a->elems [3][2] * b->elems [2][3] + a->elems [3][3] * b->elems [3][3];
}

void matrix_mul4v (v4* res, mat4* m, v4* v) {
	res->x = m->elems [0][0] * v->x + m->elems [0][1] * v->y + m->elems [0][2] * v->z + m->elems [0][3] * v->w;
	res->y = m->elems [1][0] * v->x + m->elems [1][1] * v->y + m->elems [1][2] * v->z + m->elems [1][3] * v->w;
	res->z = m->elems [2][0] * v->x + m->elems [2][1] * v->y + m->elems [2][2] * v->z + m->elems [2][3] * v->w;
	res->w = m->elems [3][0] * v->x + m->elems [3][1] * v->y + m->elems [3][2] * v->z + m->elems [3][3] * v->w;
}