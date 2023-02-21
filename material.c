#include "material.h"
#include "scene.h"
#include "vector.h"
#include "camera.h"
#include "rng.h"
#include "render_args.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define FALLOFF_CONST 10
#define BOUNCE_FALLOFF .5

material* material_init (void* loc, uint32_t color, double falloff, void (*ray_func) (collision_info*, collision_info*, void*, uint64_t*)) {
	
	//Convert color to v3 and store
	material* mat = (material*)loc;
	int r = (color & 0xFF0000) >> 16;
	int g = (color & 0x00FF00) >> 8;
	int b = (color & 0x0000FF);
	(mat->color).x = (double)r / 255;
	(mat->color).y = (double)g / 255;
	(mat->color).z = (double)b / 255;
	
	//Store the falloff
	mat->falloff = falloff;
	
	//Init ray bounce function
	mat->ray_func = ray_func;
	
	return mat;

}

material* material_init_diffuse (void* loc, uint32_t color, double falloff) {
	return material_init (loc, color, falloff, bounce_diffuse);
}

material* material_init_specular (void* loc, uint32_t color, double falloff, double fuzz) {
	material* mat = material_init (loc, color, falloff, bounce_specular);
	material_specular* mat_specular = (material_specular*)mat;
	mat_specular->fuzz = fuzz;
	return mat;
}

material* material_init_dielectric (void* loc, uint32_t color, double falloff, double refractive_index) {
	material* mat = material_init (loc, color, falloff, refract_dielectric);
	material_dielectric* mat_dielectric = (material_dielectric*)mat;
	mat_dielectric->refractive_index = refractive_index;
	return mat;
}

void rand_unitsphere (v3* loc, uint64_t* seed) {
	do {
		loc->x = 2 * (myrng (seed) / (MYRNG_MAX + 1.0)) - 1;
		loc->y = 2 * (myrng (seed) / (MYRNG_MAX + 1.0)) - 1;
		loc->z = 2 * (myrng (seed) / (MYRNG_MAX + 1.0)) - 1;
	} while (mag3 (loc) > 1);
}

int is_degenerate (v3* v) {
	if ((v->x <= 0 && v->x > -BOUNCE_EPSILON) || (v->x >= 0 && v->x < BOUNCE_EPSILON) ||
		(v->y <= 0 && v->y > -BOUNCE_EPSILON) || (v->y >= 0 && v->y < BOUNCE_EPSILON) ||
		(v->x <= 0 && v->z > -BOUNCE_EPSILON) || (v->z >= 0 && v->z < BOUNCE_EPSILON))
		{
			return 1;
		}
	return 0;
}

void bounce_none (collision_info* res, collision_info* start, void* scene_ptr, uint64_t* seed) {
	*res = *start;
}

void bounce_diffuse (collision_info* res, collision_info* start, void* scene_ptr, uint64_t* seed) {
	
	//Base case
	if (start->depth > get_global_args ()->max_depth) {
		*res = *start;
		initv3 (&(res->color), 0, 0, 0);
		return;
	}
	
	//Recursive case
	v3 rv, offs, vdir;
	do {
		rand_unitsphere (&rv, seed);
	} while (is_degenerate (&rv));
	add3 (&offs, &(start->pos), &(start->normal));
	add3 (&offs, &offs, &rv);
	diff3 (&vdir, &offs, &(start->pos));
	normalize3 (&vdir, &vdir);
	cast (res, start, &vdir, scene_ptr, seed, start->depth + 1);
	
	if (res->collided) {
		
		//Get the hit object
		scene_obj* hit_obj = res->collided_obj;
		
		diff3 (&offs, &(start->pos), &(res->pos)); //Different offset, same offs
		scale3 (&(res->color), &(res->color), ((material*)(hit_obj->mat))->falloff);
		res->color.x *= start->color.x;
		res->color.y *= start->color.y;
		res->color.z *= start->color.z;
		
	} else {
		res->color = *(get_sky ());
		res->color.x *= start->color.x;
		res->color.y *= start->color.y;
		res->color.z *= start->color.z;
	}
	
}

void bounce_specular (collision_info* res, collision_info* start, void* scene_ptr, uint64_t* seed) {
	
	//Base case
	if (start->depth > get_global_args ()->max_depth) {
		*res = *start;
		initv3 (&(res->color), 0, 0, 0);
		return;
	}
	
	//Get the material
	material_specular* mat_s = ((scene_obj*)(start->collided_obj))->mat;
	
	//Recursive case
	//Calculate reflected vector
	v3 offs, vdir;
	add3 (&offs, &(start->pos), &(start->dir));
	v3 o, dir, ppt, pn;
	o = offs;
	dir = (start->normal);
	ppt = start->pos;
	pn = start->normal;
	double sln = (pn.x * (ppt.x - o.x)
		+ pn.y * (ppt.y - o.y)
		+ pn.z * (ppt.z - o.z)) /
		(pn.x * dir.x
		+ pn.y * dir.y
		+ pn.z * dir.z);
	sln *= 2; //Reflect
	scale3 (&dir, &dir, sln);
	add3 (&vdir, &o, &dir);
	diff3 (&dir, &vdir, &(start->pos));
	
	//Add fuzz factor
	rand_unitsphere (&offs, seed);
	scale3 (&offs, &offs, mat_s->fuzz);
	add3 (&dir, &dir, &offs);
	normalize3 (&dir, &dir);
	
	start->inside_obj = (void*)0;
	cast (res, start, &dir, scene_ptr, seed, start->depth + 1);
	
	if (res->collided) {
		//TODO falloff?
		res->color.x *= start->color.x;
		res->color.y *= start->color.y;
		res->color.z *= start->color.z;
	} else {
		res->color = *(get_sky ());
		res->color.x *= start->color.x;
		res->color.y *= start->color.y;
		res->color.z *= start->color.z;
	}
	
}

void refract_dielectric (collision_info* res, collision_info* start, void* scene_ptr, uint64_t* seed) {
	
	//Base case
	if (start->depth > get_global_args ()->max_depth) {
		*res = *start;
		initv3 (&(res->color), 0, 0, 0);
		return;
	}
	
	//Get the material
	material_dielectric* mat_d = ((scene_obj*)(start->collided_obj))->mat;
	
	//Recursive case
	//Check to reflect/refract
	double rir = start->inside_obj ? mat_d->refractive_index / 1 : 1 / mat_d->refractive_index;
	//cos_theta = -r dot n
	v3 negative_r;
	scale3 (&negative_r, &(start->dir), -1);
	double cos_theta = dot3 (&negative_r, &(start->normal));
	//Calculate sin_theta
	double sin_theta = sqrt (1 - cos_theta * cos_theta);
	//Check for TIR
	int reflect = 0;
	if (rir * sin_theta > 1) {
		reflect = 1;
	} else {
		//Calculate schlict's approx.
		double param_t = (1 - rir) / (1 + rir);
		param_t = param_t * param_t;
		double pr = param_t + (1 - param_t) * (1 - cos_theta) * (1 - cos_theta) * (1 - cos_theta) * (1 - cos_theta) * (1 - cos_theta);
		double rn = ((double)myrng (seed)) / MYRNG_MAX;
		if (rn < pr) {
			reflect = 1;
		}
	}
	
	if (reflect) {
		//Reflection case (copy-pasted from bounce_specular)
		v3 offs, vdir;
		add3 (&offs, &(start->pos), &(start->dir));
		v3 o, dir, ppt, pn;
		o = offs;
		dir = (start->normal);
		ppt = start->pos;
		pn = start->normal;
		double sln = (pn.x * (ppt.x - o.x)
			+ pn.y * (ppt.y - o.y)
			+ pn.z * (ppt.z - o.z)) /
			(pn.x * dir.x
			+ pn.y * dir.y
			+ pn.z * dir.z);
		sln *= 2; //Reflect
		scale3 (&dir, &dir, sln);
		add3 (&vdir, &o, &dir);
		diff3 (&dir, &vdir, &(start->pos));
		
		//Add fuzz factor
		rand_unitsphere (&offs, seed);
		scale3 (&offs, &offs, 0);
		add3 (&dir, &dir, &offs);
		normalize3 (&dir, &dir);
		
		start->inside_obj = (void*)0;
		cast (res, start, &dir, scene_ptr, seed, start->depth + 1);
		
	} else {
		//Refraction case
		//rperp = rir * (r + n * cos_theta)
		v3 rperp, rprime, nscl;
		scale3 (&nscl, &(start->normal), cos_theta);
		add3 (&rperp, &nscl, &(start->dir));
		scale3 (&rperp, &rperp, rir);
		
		//r' = rperp - n * sqrt (1.0 - rperp dot rperp)
		scale3 (&nscl, &(start->normal), sqrt (1.0 - dot3 (&rperp, &rperp)));
		diff3 (&rprime, &rperp, &nscl);
		
		if (start->inside_obj == start->collided_obj) {
			//printf ("EXITED SPHERE\n");
			start->inside_obj = (void*)0;
		} else {
			start->inside_obj = start->collided_obj;
		}
		cast (res, start, &rprime, scene_ptr, seed, start->depth + 1);
		
	}
	
	if (res->collided) {
		//TODO falloff?
		res->color.x *= start->color.x;
		res->color.y *= start->color.y;
		res->color.z *= start->color.z;
	} else {
		res->color = *(get_sky ());
		res->color.x *= start->color.x;
		res->color.y *= start->color.y;
		res->color.z *= start->color.z;
	}
	
}

void hit_lightsource (collision_info* res, collision_info* start, void* scene_ptr, uint64_t* seed) {
	res->color.x = 2;
	res->color.y = 2;
	res->color.z = 2;
}