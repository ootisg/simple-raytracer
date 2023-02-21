#include "vertex.h"

#include <stdio.h>
#include <stdlib.h>

void printv1 (v1* v) {
	printf ("[%f]", v->x);
}

void printv2 (v2* v) {
	printf ("[%f, %f]", v->x, v->y);
}

void printv3 (v3* v) {
	printf ("[%f, %f, %f]", v->x, v->y, v->z);
}

void printv4 (v4* v) {
	printf ("[%f, %f, %f, %f]", v->x, v->y, v->z, v->w);
}

v1* initv1 (void* loc, double x) {
	v1* v = (v1*)loc;
	v->x = x;
	return v;
}

v2* initv2 (void* loc, double x, double y) {
	v2* v = (v2*)loc;
	v->x = x;
	v->y = y;
	return v;
}

v3* initv3 (void* loc, double x, double y, double z) {
	v3* v = (v3*)loc;
	v->x = x;
	v->y = y;
	v->z = z;
	return v;
}

v4* initv4 (void* loc, double x, double y, double z, double w) {
	v4* v = (v4*)loc;
	v->x = x;
	v->y = y;
	v->z = z;
	v->w = w;
	return v;
}

v1* newv1 (double x) {
	return initv1 (malloc (sizeof (v1)), x);
}

v2* newv2 (double x, double y) {
	return initv2 (malloc (sizeof (v2)), x, y);
}

v3* newv3 (double x, double y, double z) {
	return initv3 (malloc (sizeof (v3)), x, y, z);
}

v4* newv4 (double x, double y, double z, double w) {
	return initv4 (malloc (sizeof (v4)), x, y, z, w);
}