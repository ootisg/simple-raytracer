#ifndef SCENE_H
#define SCENE_H

#include "vertex.h"
#include "tri.h"
#include "material.h"

#include <stdint.h>

#define SCENE_OBJ_TYPE_NONE -1
#define SCENE_OBJ_TYPE_SPHERE 0
#define SCENE_OBJ_TYPE_TRI 1

#define SCENE_MAX_OBJS 1024

struct scene_obj {
	int obj_type;
	v3 pos;
	void* hit;
	void* mat;
	v3 data[16]; //Scene objects have enough data to internally store 16 v3 for additional data
};

typedef struct scene_obj scene_obj;

struct scene_obj_sphere {
	int obj_type;
	v3 pos;
	void* hit;
	void* mat;
	double radius;
};

typedef struct scene_obj_sphere scene_obj_sphere;

struct scene_obj_tri {
	int obj_type;
	v3 pos;
	void* hit;
	void* mat;
	tri geom;
};

typedef struct scene_obj_tri scene_obj_tri;

struct scene {
	int num_objs;
	scene_obj* objs;
};

typedef struct scene scene;

scene* scene_init (void* loc);
uint8_t get_red (uint32_t color);
uint8_t get_green (uint32_t color);
uint8_t get_blue (uint32_t color);

void scene_add (scene* s, scene_obj* obj);
scene_obj* scene_alloc_obj (scene* s);

scene_obj_sphere* sphere_init (void* loc, material* mat, v3* pos, double radius);
scene_obj_tri* scene_tri_init (void* loc, material* mat, tri* geom);

#endif