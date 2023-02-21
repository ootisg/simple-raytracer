#include "render_args.h"

#include <string.h>

render_args* global_args;

render_args* get_render_args (void* loc, int argc, char* argv[]) {
	
	//Get the struct at loc
	render_args* args = (render_args*)loc;
	
	//Set defaults
	args->num_threads = DEFAULT_NUM_THREADS;
	args->img_width = DEFAULT_IMG_WIDTH;
	args->img_height = DEFAULT_IMG_HEIGHT;
	args->num_samples = DEFAULT_NUM_SAMPLES;
	args->max_depth = DEFAULT_MAX_DEPTH;
	args->fixed_cam = 0;
	args->in_path = DEFAULT_IN_PATH;
	args->out_path = DEFAULT_OUT_PATH;
	
	//Populate the args
	int i = 0;
	int threads_use_line_width = 0;
	for (i = 0; i < argc; i++) {
		int arglen = strlen (argv[i]);
		if (arglen >= 2) {
			char* argstr = argv[i] + 2;
			if (argv[i][0] == '-' && argv[i][1] == 't') {
				if (arglen == 2) {
					threads_use_line_width = 1;
				} else {
					args->num_threads = atoi (argstr);
				}
			}
			if (argv[i][0] == '-' && argv[i][1] == 'w') {
				args->img_width = atoi (argstr);
			}
			if (argv[i][0] == '-' && argv[i][1] == 'h') {
				args->img_height = atoi (argstr);
			}
			if (argv[i][0] == '-' && argv[i][1] == 's') {
				args->num_samples = atoi (argstr);
			}
			if (argv[i][0] == '-' && argv[i][1] == 'd') {
				args->max_depth = atoi (argstr);
			}
			if (argv[i][0] == '-' && argv[i][1] == 'i') {
				args->in_path = argstr;
			}
			if (argv[i][0] == '-' && argv[i][1] == 'o') {
				args->out_path = argstr;
			}
		}
	}
	if (threads_use_line_width) {
		args->num_threads = args->img_width;
	}
	
	//Return args
	return args;
	
}

render_args* get_global_args () {
	return global_args;
}

void set_global_args (render_args* args) {
	global_args = args;
}