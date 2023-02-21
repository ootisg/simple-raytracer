#include "scene.h"

#include <stdlib.h>

scene* scene_init (void* loc) {
	scene* s = (scene*)loc;
	s->num_objs = 0;
	s->objs = malloc (sizeof (scene_obj) * SCENE_MAX_OBJS);
	return s;
}

scene_obj* scene_alloc_obj (scene* s) {
	return &(s->objs[(s->num_objs)++]);
}

void scene_add (scene* s, scene_obj* obj) {
	s->objs[(s->num_objs)++] = *obj;
}

scene_obj_sphere* sphere_init (void* loc, material* mat, v3* pos, double radius) {
	scene_obj_sphere* s = (scene_obj_sphere*)loc;
	s->obj_type = SCENE_OBJ_TYPE_SPHERE;
	s->pos = *pos;
	s->mat = mat;
	s->radius = radius;
	return s;
}

scene_obj_tri* scene_tri_init (void* loc, material* mat, tri* geom) {
	scene_obj_tri* t = (scene_obj_tri*)loc;
	t->obj_type = SCENE_OBJ_TYPE_TRI;
	t->mat = mat;
	t->geom = *geom;
	return t;
}