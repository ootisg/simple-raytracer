#ifndef TRI_H
#define TRI_H

#include "vertex.h"

struct tri {
	v3 a;
	v3 b;
	v3 c;
	v3 n;
};

typedef struct tri tri;

//inittri, newtri perform a deep copy of the vertices they are given
tri* inittri (void* loc, v3* a, v3* b, v3* c);
tri* newtri (v3* a, v3* b, v3* c);

void printtri (tri* t);

v3* normal (void* loc, v3* a, v3* b, v3* c);

v3* barycentric (void* loc, v3* pt, tri* t);

#endif