#include "vector.h"
#include "tri.h"
#include "camera.h"
#include "tri.h"
#include "render_args.h"
#include "render.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

void render_tri (uint8_t* img_buffer, int width, int height) {
	
	//Init the tri and pt
	v3 a, b, c, wp, bc;
	tri wt;
	initv3 (&a, .3, .3, 0);
	initv3 (&b, .1, .7, 0);
	initv3 (&c, .8, .6, 0);
	inittri (&wt, &a, &b, &c);
	initv3 (&wp, 0, 0, 0);
	
	int wx, wy;
	for (wy = 0; wy < height; wy++) {
		for (wx = 0; wx < width; wx++) {
			//Calculate the pixel color
			wp.x = (double)(wx) / width;
			wp.y = (double)(wy) / height;
			barycentric (&bc, &wp, &wt);
			int r, g, b;
			if (bc.x < 0 || bc.y < 0 || bc.z < 0) {
				r = 0;
				g = 0;
				b = 0;
			} else {
				r = 255 * bc.x;
				g = 255 * bc.y;
				b = 255 * bc.z;
			}
			//Render the pixel
			int idx = (wy * width + wx) * 3;
			img_buffer [idx] = r;
			img_buffer [idx + 1] = g;
			img_buffer [idx + 2] = b;
		}
	}
}

void load_render_file (char* path) {
	
	/*//Open render file
	FILE* f = fopen (path, "r");
	
	//Initialize material buffers
	char* mat_ids = malloc (1024 * 1024);
	material* materials = malloc (sizeof (material) * 1024);
	
	//Read file
	char line[4096];
	int end_of_file = 0;
	while (!end_of_file) {
		
		//Read the current line
		int i;
		for (i = 0; i < 4096; i++) {
			int c = fgetc (f);
			if (c == EOF) {
				end_of_file = 1;
				line [i] = 0;
				break;
			}
			if (c == '\n') {
				line [i] = 0;
				break;
			}
			if (c != '\r') {
				line [i] = c;
			}
		}
		
		//TODO implement this later, it's not important
		
	}*/
	
}

void make_scene (scene* sc) {
	
	//Init the tri and pt
	v3 a, b, c;
	tri wt;
	double theta = 5.5 * M_PI / 6;
	double rad = 10;
	initv3 (&c, rad * cos(theta), -1, rad * sin(theta) + 2);
	initv3 (&b, rad * cos(theta + 2 * M_PI / 3), -1, rad * sin(theta + 2 * M_PI / 3) + 2);
	initv3 (&a, rad * cos(theta + 4 * M_PI / 3), -1, rad * sin(theta + 4 * M_PI / 3) + 2);
	inittri (&wt, &a, &b, &c);
	
	//material* green = material_init (malloc (sizeof (material)), 0x0000C000, .95, bounce_diffuse);
	//material* emissive = material_init (malloc (sizeof (material)), 0x00FFFFFF, 1.0, hit_lightsource);
	material* white = material_init (malloc (sizeof (material)), 0x00E0E0E0, .95, bounce_diffuse);
	material* red = material_init (malloc (sizeof (material)), 0x00FF0000, .95, bounce_diffuse);
	material* blue = material_init (malloc (sizeof (material)), 0x000000D0, .95, bounce_diffuse);
	material* purple = material_init (malloc (sizeof (material)), 0x00803080, .95, bounce_diffuse);
	material* glass = material_init_dielectric (malloc (sizeof (material)), 0x00E0E0E0, .99, 1.3);
	material* mirror = material_init_specular (malloc (sizeof (material)), 0x00E0E0E0, .99, 0);
	sphere_init ((scene_obj_sphere*)scene_alloc_obj (sc), glass, newv3 (0, .01, 2), 1);
	sphere_init ((scene_obj_sphere*)scene_alloc_obj (sc), red, newv3 (-.6, -.7, 1), .3);
	sphere_init ((scene_obj_sphere*)scene_alloc_obj (sc), blue, newv3 (1.9, 1.1, 4.4), 2);
	sphere_init ((scene_obj_sphere*)scene_alloc_obj (sc), mirror, newv3 (2.3, .2, 1.8), .4);
	sphere_init ((scene_obj_sphere*)scene_alloc_obj (sc), purple, newv3 (-1.8, .8, 2), .6);
	scene_tri_init ((scene_obj_tri*)scene_alloc_obj (sc), white, &wt);
	
}

void setup_camera (camera* cam) {
	
	//Change the camera view parameters here
	v3 camera_position, camera_rotation;
	initv3 (&camera_position, 0, 0, 0);
	initv3 (&camera_rotation, 0, 0, 0);
	camera_move (cam, &camera_position, &camera_rotation);
	
}

int main (int argc, char* argv[]) {
	
	//Load render args
	render_args* global_rargs = malloc (sizeof (render_args));
	get_render_args (global_rargs, argc, argv);
	set_global_args (global_rargs);
	int OUTPUT_IMG_WIDTH = global_rargs->img_width;
	int OUTPUT_IMG_HEIGHT = global_rargs->img_height;
	
	//Output render args
	printf ("Rendering scene at resolution %dx%d, with recursive depth %d and sampling rate of %d rays per pixel.\n", global_rargs->img_width, global_rargs->img_height, global_rargs->max_depth, global_rargs->num_samples);
	printf ("Number of render threads: %d\n", global_rargs->num_threads);
	
	//Init the scene
	camera* cam = camera_init (malloc (sizeof (camera)));
	scene* sc = scene_init (malloc (sizeof (scene)));
	make_scene (sc);
	
	//Render the scene
	uint8_t* img_buffer = malloc (OUTPUT_IMG_WIDTH * OUTPUT_IMG_HEIGHT * 3 * sizeof (uint32_t));
	render_frame (img_buffer, OUTPUT_IMG_WIDTH, OUTPUT_IMG_HEIGHT, cam, sc, 1);
	write_img (global_rargs->out_path, img_buffer, OUTPUT_IMG_WIDTH, OUTPUT_IMG_HEIGHT);
	
	//Tell the user the file has been rendered
	printf ("Render finished. Output image written to %s.", global_rargs->out_path);
	
	return 0;
	
}