#include "camera.h"
#include "scene.h"
#include "vector.h"
#include "material.h"
#include "rng.h"
#include "render_args.h"
#include "matrix.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <pthread.h>

#define CAMERA_MAX_DIST 10000
#define CAMERA_VIEW_WIDTH 1.5

#define SURFACE_EPSILON 0.0001

double y_scale;
v3* sky_color = 0;

pthread_t* threads;
volatile int* threads_status;
uint64_t* px_seeds;

struct render_px_info {
	uint8_t* imgbuffer;
	int img_width;
	int px_x;
	int px_y;
	camera* cam;
	scene* sc;
	v3* dir_vecs;
	int thread_id;
	uint64_t* seed;
};

v3* get_sky () {
	if (!sky_color) {
		sky_color = newv3 (.5, 1, 1);
	}
	return sky_color;
}

camera* camera_init (void* loc) {
	camera* c = (camera*)loc;
	initv3 (&(c->pos), 0, 0, 0);
	initv3 (&(c->dir), 0, 0, 0);
	camera_move (c, &(c->pos), &(c->dir));
	return c;
}

void camera_move (camera* cam, v3* start_pt, v3* rot) {
	cam->pos = *start_pt;
	//TODO calculate frame dynamically
	v4 cam_tl, cam_tr, cam_bl, cam_br;
	
	//Test with rotation matrices
	double cam_rx = CAMERA_VIEW_WIDTH;
	double cam_ry = cam_rx * (((double)get_global_args ()->img_height) / get_global_args ()->img_width);
	initv4 (&cam_tl, -cam_rx, -cam_ry, 1, 1);
	initv4 (&cam_tr, cam_rx, -cam_ry, 1, 1);
	initv4 (&cam_bl, -cam_rx, cam_ry, 1, 1);
	initv4 (&cam_br, cam_rx, cam_ry, 1, 1);
	mat4 mat_rotx, mat_roty, mat_rotz, mat_trans;
	matrix_rotx4 (&mat_rotx, rot->x);
	matrix_roty4 (&mat_roty, rot->y);
	matrix_rotz4 (&mat_rotz, rot->z);
	matrix_translate4 (&mat_trans, start_pt->x, start_pt->y, start_pt->z);
	
	//I was having issues with matrix multiplication order
	v4 res1, res2;
	matrix_mul4v (&res1, &mat_rotz, &cam_tl);
	matrix_mul4v (&res2, &mat_roty, &res1);
	matrix_mul4v (&res1, &mat_rotx, &res2);
	#ifdef FIXED_CAM
	matrix_mul4v (&res2, &mat_trans, &res1);
	#endif
	initv3 (&(cam->frame[0]), res1.x, res1.y, res1.z);
	matrix_mul4v (&res1, &mat_rotz, &cam_tr);
	matrix_mul4v (&res2, &mat_roty, &res1);
	matrix_mul4v (&res1, &mat_rotx, &res2);
	#ifdef FIXED_CAM
	matrix_mul4v (&res2, &mat_trans, &res1);
	#endif
	initv3 (&(cam->frame[1]), res1.x, res1.y, res1.z);
	matrix_mul4v (&res1, &mat_rotz, &cam_bl);
	matrix_mul4v (&res2, &mat_roty, &res1);
	matrix_mul4v (&res1, &mat_rotx, &res2);
	#ifdef FIXED_CAM
	matrix_mul4v (&res2, &mat_trans, &res1);
	#endif
	initv3 (&(cam->frame[2]), res1.x, res1.y, res1.z);
	matrix_mul4v (&res1, &mat_rotz, &cam_br);
	matrix_mul4v (&res2, &mat_roty, &res1);
	matrix_mul4v (&res1, &mat_rotx, &res2);
	#ifdef FIXED_CAM
	matrix_mul4v (&res2, &mat_trans, &res1);
	#endif
	initv3 (&(cam->frame[3]), res1.x, res1.y, res1.z);
	
}

void* render_px (void* args) {
	
	//Get arguments
	struct render_px_info* targs = (struct render_px_info*)args;
	uint8_t* imgbuffer = targs->imgbuffer;
	int img_width = targs->img_width;
	int px_x = targs->px_x;
	int px_y = targs->px_y;
	camera* cam = targs->cam;
	scene* sc = targs->sc;
	v3* dir_vecs = targs->dir_vecs;
	int thread_id = targs->thread_id;
	uint64_t seed = *(targs->seed);
	//Static memory
	v3 end_color;
	initv3 (&end_color, 0, 0, 0);
	v3 v_lerp_1, v_lerp_2, h_lerp;
	collision_info col_info;
	
	int RAYS_PER_PX = get_global_args ()->num_samples;
	
	int ri;
	collision_info origin_info;
	for (ri = 0; ri < RAYS_PER_PX; ri++) {
		
		//Get vector
		v3 cast_dir;
		y_scale = rand () / (RAND_MAX + 1.0);
		lerp3 (&v_lerp_1, &(dir_vecs [px_y * (img_width + 1) + px_x]), &(dir_vecs [(px_y + 1) * (img_width + 1) + px_x]), y_scale);
		lerp3 (&v_lerp_2, &(dir_vecs [px_y * (img_width + 1) + (px_x + 1)]), &(dir_vecs [(px_y + 1) * (img_width + 1) + (px_x + 1)]), y_scale);
		double x_scale = rand () / ((RAND_MAX) + 1.0);
		lerp3 (&h_lerp, &v_lerp_1, &v_lerp_2, x_scale);
		normalize3 (&cast_dir, &h_lerp);

		//Cast the vector
		origin_info.pos = cam->pos;
		origin_info.collided_obj = (void*)0;
		origin_info.inside_obj = (void*)0;
		cast (&col_info, &origin_info, &cast_dir, sc, &seed, 0);
		add3 (&end_color, &end_color, &(col_info.color));
		
	}
	
	//Get the final color and write the pixel
	scale3 (&end_color, &end_color, (double)1 / RAYS_PER_PX);
	if (end_color.x > 1) {end_color.x = 1;}
	if (end_color.y > 1) {end_color.y = 1;}
	if (end_color.z > 1) {end_color.z = 1;}
	int idx = (px_y * img_width + px_x) * 3;
	imgbuffer [idx] = (uint8_t)(end_color.x * 255);
	imgbuffer [idx + 1] = (uint8_t)(end_color.y * 255);
	imgbuffer [idx + 2] = (uint8_t)(end_color.z * 255);
	
	//Set thread to available
	threads_status[thread_id] = 0;
	
	//Return from execution
	if (get_global_args ()->num_threads == 1) {
		return 0;
	} else {
		pthread_exit ((void*)0);
	}

	//If not yet returned
	return 0;
	
}

tri* camera_get_geom (camera* cam, tri* tris) {
	inittri (&(tris[0]), &(cam->pos), &(cam->frame[0]), &(cam->frame[1]));
	inittri (&(tris[1]), &(cam->pos), &(cam->frame[1]), &(cam->frame[3]));
	inittri (&(tris[2]), &(cam->pos), &(cam->frame[3]), &(cam->frame[2]));
	inittri (&(tris[3]), &(cam->pos), &(cam->frame[2]), &(cam->frame[0]));
	return tris;
}

void render_frame (uint8_t* imgbuffer, int img_width, int img_height, camera* cam, scene* sc, int progress_msg) {
	
	//Initialize the thread buffers
	int MAX_RENDER_THREADS = get_global_args ()->num_threads;
	threads = malloc (sizeof (pthread_t) * MAX_RENDER_THREADS);
	threads_status = malloc (sizeof (int) * MAX_RENDER_THREADS);
	
	//Populate the direction vectors
	v3 v_lerp_1, v_lerp_2, h_lerp;
	v3* dir_vecs = malloc (sizeof (v3) * (img_width + 1) * (img_height + 1));
	int wy, wx;
	for (wy = 0; wy < img_height + 1; wy++) {
		y_scale = (double)wy / img_height;
		lerp3 (&v_lerp_1, &(cam->frame[0]), &(cam->frame[2]), y_scale);
		lerp3 (&v_lerp_2, &(cam->frame[1]), &(cam->frame[3]), y_scale);
		for (wx = 0; wx < img_width + 1; wx++) {
			double x_scale = (double)wx / img_width;
			lerp3 (&h_lerp, &v_lerp_1, &v_lerp_2, x_scale);
			normalize3 (&(dir_vecs [wy * (img_width + 1) + wx]), &h_lerp);
		}
	}
	
	/*//Populate the pixel seeds
	for (wy = 0; wy < img_height; wy++) {
		for (wx = 0; wx < img_width; wx++) {
			px_seeds [wx * img_width + wy] = get_rand_seed ();
		}
	}*/
	
	//Cast the rays
	int t_id;
	for (t_id = 0; t_id < MAX_RENDER_THREADS; t_id++) {
		threads_status [t_id] = 0;
	}
	t_id = 0;
	for (wy = 0; wy < img_height; wy++) {
		for (wx = 0; wx < img_width; wx++) {
			if (MAX_RENDER_THREADS == 1) {
				
				//Populate the render px struct and render the pixel
				struct render_px_info args;
				args.imgbuffer = imgbuffer;
				args.img_width = img_width;
				args.px_x = wx;
				args.px_y = wy;
				args.cam = cam;
				args.sc = sc;
				args.dir_vecs = dir_vecs;
				args.thread_id = 1;
				uint64_t* seed = malloc (sizeof (uint64_t));
				*seed = get_rand_seed ();
				args.seed = seed;
				render_px (&args);
				
			} else {
				//Populate the render px struct and render the pixel
				struct render_px_info* args = malloc (sizeof (struct render_px_info));
				(*args).imgbuffer = imgbuffer;
				(*args).img_width = img_width;
				(*args).px_x = wx;
				(*args).px_y = wy;
				(*args).cam = cam;
				(*args).sc = sc;
				(*args).dir_vecs = dir_vecs;
				(*args).thread_id = t_id;
				uint64_t* seed = (uint64_t*)malloc (sizeof (uint64_t));
				*seed = get_rand_seed ();
				(*args).seed = seed;
				pthread_t pthread_id;
				threads_status [t_id] = 1;
				//printf ("CREATING THREAD %d\n", t_id);
				pthread_create (&pthread_id, NULL, render_px, args);
				threads [t_id] = pthread_id;
				
				//Ensure we don't exceed the maximum # of threads
				t_id++;
				if (t_id >= MAX_RENDER_THREADS) {
					for (t_id = 0; t_id < MAX_RENDER_THREADS; t_id++) {
						void* thread_ret;
						pthread_join (threads [t_id], &thread_ret);
					}
					t_id = 0;
				}
				
			}
		}
		if (MAX_RENDER_THREADS > 1) {
			for (t_id = 0; t_id < MAX_RENDER_THREADS; t_id++) {
				void* thread_ret;
				pthread_join (threads [t_id], &thread_ret);
			}
		}
		t_id = 0;
		if (progress_msg) {
			printf ("Rendered line %d of %d\n", wy + 1, img_height);
		}
	}
	
}

void write_img (char* filename, uint8_t* imgbuffer, int img_width, int img_height) {
	FILE* f = fopen (filename, "wb");
	char buf[128];
	sprintf (buf, "P6 %d %d 255\n", img_width, img_height);
	fwrite (buf, strlen (buf), 1, f);
	int wx, wy;
	for (wy = 0; wy < img_height; wy++) {
		for (wx = 0; wx < img_width; wx++) {
			int idx = (wy * img_width + wx) * 3;
			fwrite (imgbuffer + idx, 1, 1, f);
			fwrite (imgbuffer + idx + 1, 1, 1, f);
			fwrite (imgbuffer + idx + 2, 1, 1, f);
		}
	}
	fclose (f);
}

uint32_t cast (collision_info* loc, collision_info* origin, v3* dir, scene* sc, uint64_t* seed, int depth) {
	
	v3* o = &(origin->pos);
	int num_objs = sc->num_objs;
	scene_obj* objs = sc->objs;
	int i;
	double dist = CAMERA_MAX_DIST;
	collision_info working;
	scene_obj* collided_obj;
	loc->collided = 0;
	for (i = 0; i < num_objs; i++) {
		scene_obj* curr = &(objs[i]);
		collide (&working, origin, dir, sc, curr, seed, depth);
		v3 diffv;
		diff3 (&diffv, &(working.pos), o);
		double currdist;
		if (working.collided && (currdist = mag3 (&diffv)) < dist) {
			if (currdist > SURFACE_EPSILON) {
				dist = currdist;
				*loc = working;
				collided_obj = curr;
			}
		}
	}
	if (loc->collided) {
		//Determine the color of the material
		collision_info rec_info;
		loc->depth = depth;
		((material*)(collided_obj->mat))->ray_func (&rec_info, loc, sc, seed);
		loc->color = rec_info.color; //TODO VERY UNSURE IF THIS IS THE CORRECT LOGIC
		return 0;
	} else {
		loc->collided = 0;
		loc->color = *(get_sky ());
		return 0;
	}
}

uint32_t collide (collision_info* loc, collision_info* origin, v3* dir, scene* sc, scene_obj* obj, uint64_t* seed, int depth) {
	
	loc->collided = 0;
	v3* o = &(origin->pos);
	if (obj->obj_type == SCENE_OBJ_TYPE_SPHERE) {
		scene_obj_sphere* sphere = (scene_obj_sphere*)obj;
		v3 offs;
		diff3 (&offs, o, &(sphere->pos));
		double dir_dot_offs = dot3 (dir, &offs);
		double offssqr = dot3 (&offs, &offs);
		double rsqr = sphere->radius * sphere->radius;
		//Check for intersection
		double root_term = dir_dot_offs * dir_dot_offs - (offssqr - rsqr);
		if (root_term >= 0) {
			//Collided
			double sln1 = (-2 * dir_dot_offs + sqrt (4 * root_term)) / 2;
			double sln2 = (-2 * dir_dot_offs - sqrt (4 * root_term)) / 2;
			
			//Logic block here to make sure we hit the right spot
			double sln;
			if (sln1 < SURFACE_EPSILON && sln2 < SURFACE_EPSILON) {
				//Ray traveled backwards
				loc->collided = 0;
				return 0;
			} else if (sln1 < SURFACE_EPSILON) {
				sln = sln2;
			} else if (sln2 < SURFACE_EPSILON) {
				sln = sln1;
			} else {
				sln = sln1 >= sln2 ? sln2 : sln1;
				if (sln < .0005) return 0;
			}
			
			v3 vecsln, normal;
			scale3 (&vecsln, dir, sln);
			add3 (&vecsln, &vecsln, o);
			diff3 (&normal, &vecsln, &(sphere->pos));
			normalize3 (&normal, &normal);
			//Material call
			loc->collided = 1;
			loc->pos = vecsln;
			loc->normal = normal;
			if (origin->inside_obj == obj) {
				scale3 (&(loc->normal), &normal, -1);
				loc->inside_obj = obj;
			} else {
				loc->normal = normal;
				loc->inside_obj = (void*)0;
			}
			loc->dir = *dir;
			loc->color = ((material*)(obj->mat))->color;
			loc->collided_obj = obj;
			return 0;
		} else {
			loc->collided = 0;
			return 0;
		}
	} else if (obj->obj_type == SCENE_OBJ_TYPE_TRI) {
		
		//Ray-plane collision
		scene_obj_tri* scene_t = (scene_obj_tri*)obj;
		tri* t = &(scene_t->geom);
		double sln = ((t->n).x * ((t->a).x - o->x)
					+ (t->n).y * ((t->a).y - o->y)
					+ (t->n).z * ((t->a).z - o->z)) /
					((t->n).x * dir->x
					+ (t->n).y * dir->y
					+ (t->n).z * dir->z);
		if (sln < 0) {
			loc->collided = 0;
			return 0;
		}
		v3 vecsln; 
		scale3 (&vecsln, dir, sln);
		add3 (&vecsln, &vecsln, o);
		
		//Check if inside triangle
		v3 bary;
		barycentric (&bary, &vecsln, t);
		if (bary.x >= 0 && bary.y >= 0 && bary.z >= 0) {
			loc->collided = 1;
			loc->pos = vecsln;
			loc->normal = t->n;
			loc->dir = *dir;
			loc->color = ((material*)(obj->mat))->color;
			loc->collided_obj = obj;
			if (origin->inside_obj == obj) {
				loc->inside_obj = obj;
			} else {
				loc->inside_obj = (void*)0;
			}
			return 0;
		} else {
			loc->collided = 0;
			return 0;
		}
		
	}
	loc->collided = 0;
	loc->color = *(get_sky ());
	return 0;
	
}