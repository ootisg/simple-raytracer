#include "vector.h"

#include <math.h>
#include <stdio.h>

//Some renamed initialization funcs for convenience
v1* vec1 (double x) {
	return newv1 (x);
}

v2* vec2 (double x, double y) {
	return newv2 (x, y);
}

v3* vec3 (double x, double y, double z) {
	return newv3 (x, y, z);
}

v4* vec4 (double x, double y, double z, double w) {
	return newv4 (x, y, z, w);
}

//Define magnitude funcs
double mag3 (v3* v) {
	return sqrt (v->x * v->x + v->y * v->y + v->z * v->z);
}

//Define add funcs
void add3 (v3* res, v3* a, v3* b) {
	res->x = a->x + b->x;
	res->y = a->y + b->y;
	res->z = a->z + b->z;
}

//Define diff funcs
void diff3 (v3* res, v3* a, v3* b) {
	res->x = a->x - b->x;
	res->y = a->y - b->y;
	res->z = a->z - b->z;
}

//Define scale funcs
void scale3 (v3* res, v3* v, double amt) {
	res->x = v->x * amt;
	res->y = v->y * amt;
	res->z = v->z * amt;
	
}

//Define lerp funcs
void lerp3 (v3* res, v3* v1, v3* v2, double amt) {
	double invamt = 1 - amt;
	res->x = v1->x * amt + v2->x * invamt;
	res->y = v1->y * amt + v2->y * invamt;
	res->z = v1->z * amt + v2->z * invamt;
}

//Define normalize funcs
void normalize3 (v3* res, v3* v) {
	double len = mag3 (v);
	res->x = v->x / len;
	res->y = v->y / len;
	res->z = v->z / len;
}

//Define unit funcs
void unit3 (v3* res, v3* from, v3* to) {
	diff3 (res, to, from);
	normalize3 (res, res);
}

//Define dot funcs
double dot3 (v3* a, v3* b) {
	return a->x * b->x + a->y * b->y + a->z * b->z;
}

//Define cross funcs
void cross (v3* res, v3* a, v3* b) {
	res->x = a->y * b->z - a->z * b->y;
	res->y = a->z * b->x - a->x * b->z;
	res->z = a->x * b->y - a->y * b->x;
}

void crossn (v3* res, v3* a, v3* b) {
	cross (res, a, b);
	normalize3 (res, res);
}