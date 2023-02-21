#ifndef MATERIAL_H
#define MATERIAL_H

#include "vertex.h"

#include <stdint.h>

#define BOUNCE_EPSILON 0.0001

struct collision_info {
	int collided;
	int depth;
	v3 pos;
	v3 dir;
	v3 normal;
	v3 color;
	void* collided_obj;
	void* inside_obj;
};

typedef struct collision_info collision_info;

struct material {
	v3 color;
	double falloff;
	void (*ray_func) (collision_info*, collision_info*, void*, uint64_t*);
	char params[1024];
};

struct material_specular {
	v3 color;
	double falloff;
	void (*ray_func) (collision_info*, collision_info*, void*, uint64_t*);
	double fuzz;
};

struct material_dielectric {
	v3 color;
	double falloff;
	void (*ray_func) (collision_info*, collision_info*, void*, uint64_t*);
	double refractive_index;
};

typedef struct material material;
typedef struct material_specular material_specular;
typedef struct material_dielectric material_dielectric;

material* material_init (void* loc, uint32_t color, double falloff, void (*ray_func) (collision_info*, collision_info*, void*, uint64_t*));
material* material_init_diffuse (void* loc, uint32_t color, double falloff);
material* material_init_specular (void* loc, uint32_t color, double falloff, double fuzz);
material* material_init_dielectric (void* loc, uint32_t color, double falloff, double refractive_index);

void bounce_none (collision_info* res, collision_info* start, void* scene_ptr, uint64_t* seed);
void bounce_diffuse (collision_info* res, collision_info* start, void* scene_ptr, uint64_t* seed);
void bounce_specular (collision_info* res, collision_info* start, void* scene_ptr, uint64_t* seed);
void refract_dielectric (collision_info* res, collision_info* start, void* scene_ptr, uint64_t* seed);
void hit_lightsource (collision_info* res, collision_info* start, void* scene_ptr, uint64_t* seed);

#endif