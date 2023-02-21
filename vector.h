#ifndef VECTOR_H
#define VECTOR_H

#include "vertex.h"

//Some renamed initialization funcs for convenience
v1* vec1 (double x);
v2* vec2 (double x, double y);
v3* vec3 (double x, double y, double z);
v4* vec4 (double x, double y, double z, double w);

//Define magnitude funcs
double mag3 (v3* v);

//Define add funcs
void add3 (v3* res, v3* a, v3* b);

//Define diff funcs
void diff3 (v3* res, v3* a, v3* b);

//Define scale funcs
void scale3 (v3* res, v3* v, double amt);

//Define lerp funcs
void lerp3 (v3* res, v3* v1, v3* v2, double amt);

//Define normalize funcs
void normalize3 (v3* res, v3* v);

//Define unit funcs
void unit3 (v3* res, v3* from, v3* to);

//Define dot funcs
double dot3 (v3* a, v3* b);

//Define cross funcs
void cross (v3* res, v3* a, v3* b);
void crossn (v3* res, v3* a, v3* b);

#endif