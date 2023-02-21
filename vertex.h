#ifndef VERTEX_H
#define VERTEX_H

struct v1 {
	double x;
};

struct v2 {
	double x;
	double y;
};

struct v3 {
	double x;
	double y;
	double z;
};

struct v4 {
	double x;
	double y;
	double z;
	double w;
};

//Define vertex structs
typedef struct v1 v1;
typedef struct v2 v2;
typedef struct v3 v3;
typedef struct v4 v4;

//Define print functions for vertices
void printv1 (v1* v);
void printv2 (v2* v);
void printv3 (v3* v);
void printv4 (v4* v);

//Define vertex init funcs
v1* initv1 (void* loc, double x);
v2* initv2 (void* loc, double x, double y);
v3* initv3 (void* loc, double x, double y, double z);
v4* initv4 (void* loc, double x, double y, double z, double w);

//Define "new vertex" funcs
v1* newv1 (double x);
v2* newv2 (double x, double y);
v3* newv3 (double x, double y, double z);
v4* newv4 (double x, double y, double z, double w);

#endif