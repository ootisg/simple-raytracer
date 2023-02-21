#ifndef CAMERA_H
#define CAMERA_H
#include "scene.h"
#include "matrix.h"

#include <stdint.h>

struct camera {
	v3 pos;
	v3 dir;
	v3 frame[4];
};
typedef struct camera camera;

camera* camera_init (void* loc);
v3* get_sky ();
void camera_move (camera* cam, v3* start_pt, v3* look_pt);
void render_frame (uint8_t* imgbuffer, int img_width, int img_height, camera* cam, scene* sc, int progress_msg);
void write_img (char* filename, uint8_t* imgbuffer, int img_width, int img_height);
uint32_t cast (collision_info* loc, collision_info* origin, v3* dir, scene* sc, uint64_t* seed, int depth);
uint32_t collide (collision_info* loc, collision_info* origin, v3* dir, scene* sc, scene_obj* obj, uint64_t* seed, int depth);
tri* camera_get_geom (camera* cam, tri* tris);

#endif