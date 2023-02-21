#include "tri.h"
#include "vector.h"

#include <stdlib.h>
#include <stdio.h>

tri* inittri (void* loc, v3* a, v3* b, v3* c) {
	tri* t = (tri*)loc;
	(t->a).x = a->x;
	(t->a).y = a->y;
	(t->a).z = a->z;
	(t->b).x = b->x;
	(t->b).y = b->y;
	(t->b).z = b->z;
	(t->c).x = c->x;
	(t->c).y = c->y;
	(t->c).z = c->z;
	normal (&(t->n), a, b, c);
	return t;
}

tri* newtri (v3* a, v3* b, v3* c) {
	inittri (malloc (sizeof (v3)), a, b, c);
}

void printtri (tri* t) {
	printf ("tri { ");
	printv3 (&(t->a));
	printf (", ");
	printv3 (&(t->b));
	printf (", ");
	printv3 (&(t->c));
	printf (" }");
}

v3* normal (void* loc, v3* a, v3* b, v3* c) {
	v3 u, v;
	diff3 (&u, a, b);
	diff3 (&v, b, c);
	cross (loc, &u, &v);
	normalize3 (loc, (v3*)loc);
}

v3* barycentric (void* loc, v3* pt, tri* t) {
	
	//Declare the scratch vectors
	v3 wv1, wv2, wv3;
	
	//Calculate the area of the full triangle
	diff3 (&wv1, &(t->c), &(t->a));
	diff3 (&wv2, &(t->c), &(t->b));
	cross (&wv3, &wv1, &wv2);
	double area_abc = dot3 (&wv3, &(t->n)) / 2;
	
	//Calculate the area of triangle apb
	diff3 (&wv1, &(t->a), pt);
	diff3 (&wv2, &(t->b), pt);
	cross (&wv3, &wv1, &wv2);
	double area_apb = dot3 (&wv3, &(t->n)) / 2;
	//Calculate the area of triangle bpc
	diff3 (&wv1, &(t->b), pt);
	diff3 (&wv2, &(t->c), pt);
	cross (&wv3, &wv1, &wv2);
	double area_bpc = dot3 (&wv3, &(t->n)) / 2;
	//Calculate the area of triangle cpa
	diff3 (&wv1, &(t->c), pt);
	diff3 (&wv2, &(t->a), pt);
	cross (&wv3, &wv1, &wv2);
	double area_cpa = dot3 (&wv3, &(t->n)) / 2;
	
	//Cast the result vector
	v3* res = (v3*)loc;
	
	//Calculate the coords
	res->x = area_bpc / area_abc; //alpha
	res->y = area_cpa / area_abc; //beta
	res->z = area_apb / area_abc; //gamma
	return res;
	
}