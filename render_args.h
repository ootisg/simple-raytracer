#ifndef RENDER_ARGS_H
#define RENDER_ARGS_H

#define DEFAULT_NUM_THREADS 1
#define DEFAULT_IMG_WIDTH 1600
#define DEFAULT_IMG_HEIGHT 900
#define DEFAULT_NUM_SAMPLES 4
#define DEFAULT_MAX_DEPTH 16
#define DEFAULT_IN_PATH NULL
#define DEFAULT_OUT_PATH "output.ppm"

struct render_args {
	int num_threads;
	int img_width;
	int img_height;
	int num_samples;
	int max_depth;
	int fixed_cam;
	char* in_path;
	char* out_path;
};

typedef struct render_args render_args;

render_args* get_render_args (void* loc, int argc, char* argv[]);
render_args* get_global_args ();
void set_global_args (render_args* args);

#endif