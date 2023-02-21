#include "render_gl.h"

#include "inputs_gl.h"
#include "vector.h"
#include "tri.h"
#include "camera.h"
#include "tri.h"
#include "render_args.h"
#include "scene.h"
#include "matrix.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define RT_OUTPUT_WIDTH 128
#define RT_OUTPUT_HEIGHT 72

//This file is a WIP
//Probably not from this file, but there seems to be a severe memory leak.

GLfloat g_ex_vertex_buffer_data[] = { 
    -1.0f,  -1.0f,
    -1.0f,  1.0f,
     1.0f,  -1.0f,
     1.0f,  1.0f
};
GLuint g_ex_element_buffer_data[] = { 0, 1, 2, 3 };

GLfloat* g_vertex_buffer_data = g_ex_vertex_buffer_data;
GLuint* g_element_buffer_data = g_ex_element_buffer_data;

int g_vertex_buffer_fill;

int frame_count;
int frame_start_time;

char* render_img_buffer;
camera* global_cam;
camera* global_cam2;
scene* global_sc;
v3 camera_pos;
v3 camera_ang;

render_args* rargs_rt;
render_args* rargs_render;

scene_obj_tri* camtris[4];

v2* click_point;

double hrot_base;
double vrot_base;

struct {
	GLuint vertex_buffer;
	GLuint element_buffer;
	GLuint vertex_shader;
	GLuint frag_shader;
	GLuint program;
	GLuint texture;
	int refresh_texture;
	struct {
		GLuint position;
		GLuint tex;
		GLuint texid;
	} attributes;
	struct {
		GLuint texture;
	} uniforms;
} g_resources;

void refresh_all_textures () {
	g_resources.refresh_texture = 1;
}

void show_info_log(
    GLuint object,
    PFNGLGETSHADERIVPROC glGet__iv,
    PFNGLGETSHADERINFOLOGPROC glGet__InfoLog
)
{
	//COPY PASTED
    GLint log_length;
    char *log;

    glGet__iv(object, GL_INFO_LOG_LENGTH, &log_length);
    log = malloc(log_length);
    glGet__InfoLog(object, log_length, NULL, log);
    fprintf(stderr, "%s", log);
    free(log);
}

void init_vertex_buffers () {
	g_vertex_buffer_data = malloc (sizeof (GLfloat) * 1024);
	g_element_buffer_data = malloc (sizeof (GLfloat) * 1024);
}

GLchar* file_contents (const char* filename, int* length) {
	//Open file
	FILE* f = fopen (filename, "r");
	
	//Get the file length
	fseek (f, 0, SEEK_END);
	*length = ftell (f);
	fseek (f, 0, SEEK_SET);
	
	GLchar* fcontents = malloc ((*length + 1) * sizeof(GLchar));
	//Make a buffer and read the file to it
	int i;
	for (i = 0; i < *length; i++) {
		int read_char = fgetc (f);
		if (read_char == EOF) {
			*length = i;
			fcontents[i] = 0; //Add a null terminator
			break;
		}
		fcontents[i] = (GLchar)read_char;
	}
	printf ("%d\n%s\n", *length, fcontents);
	
	//Return the buffer ptr
	return fcontents;
}

GLuint make_buffer (GLenum target, const void* buffer_data, GLsizei buffer_size) {
	GLuint buffer;
	glGenBuffers (1, &buffer);
	glBindBuffer (target, buffer);
	glBufferData (target, buffer_size, buffer_data, GL_STATIC_DRAW);
	return buffer;
}

GLuint make_texture_from_buffer (char* buffer, int width, int height) {
	GLuint texture;
	glGenTextures (1, &texture);
	glBindTexture (GL_TEXTURE_2D, texture);
	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glTexImage2D (GL_TEXTURE_2D, 	//Target
				  0,				//Mipmap Level
				  GL_RGB8,			//Format
				  width, height,	//Width and Height
				  0,				//Well okay then
				  GL_RGB, 			//Ordering(?)
				  GL_UNSIGNED_BYTE,	//Type of the pixel data
				  buffer			//The image data
				  );
				  
	return texture;
}

GLuint free_texture (GLuint texture) {
	glDeleteTextures (1, &texture);
}

GLuint make_shader (GLenum type, const char* filename) {
	
	//Read in the shader
	GLint length;
	GLchar* source = file_contents (filename, &length);
	GLuint shader;
	GLuint shader_ok;
	
	//Terminate if file reading failed
	if (!source) {
		return 0;
	}
	
	//Make the shader
	shader = glCreateShader (type);
	glShaderSource (shader, 1, (const GLchar**)&source, &length);
	free (source);
	glCompileShader (shader);
	
	//Handle any shader related mishaps
	glGetShaderiv (shader, GL_COMPILE_STATUS, &shader_ok);
	if (!shader_ok) {
		//Copy-pasted, don't judge
		fprintf(stderr, "Failed to compile %s:\n", filename);
		show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
        glDeleteShader(shader);
        return 0;
	}
	//RETURN THE SHADER (IMPORTANT!)
	return shader;
}

GLuint make_program (GLuint vertex_shader, GLuint frag_shader) {
	GLint program_ok;
	GLuint program = glCreateProgram ();
	glAttachShader (program, vertex_shader);
	glAttachShader (program, frag_shader);
	glLinkProgram (program);
	//Handle any program related mishaps (copy-pasted, oof)
	glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (!program_ok) {
        fprintf(stderr, "Failed to link shader program:\n");
        show_info_log(program, glGetProgramiv, glGetProgramInfoLog);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

void refresh_buffers () {
	g_resources.vertex_buffer = make_buffer (GL_ARRAY_BUFFER, g_vertex_buffer_data, (g_vertex_buffer_fill * 2 + 2) * sizeof (GLfloat)); //Okay wtf
	g_resources.element_buffer = make_buffer (GL_ARRAY_BUFFER, g_element_buffer_data, g_vertex_buffer_fill * 1 * sizeof (GLfloat));
}

void refresh_textures () {
	if (g_resources.refresh_texture || !g_resources.texture) {
		if (g_resources.texture) {
			free_texture (g_resources.texture);
		}
		//printf ("MAKING TEXTURE\n");
		g_resources.texture = make_texture_from_buffer (render_img_buffer, RT_OUTPUT_WIDTH, RT_OUTPUT_HEIGHT);
		g_resources.refresh_texture = 0;
	}
}

int make_resources () {
	init_vertex_buffers ();
	g_vertex_buffer_fill = 4;
	memcpy (g_vertex_buffer_data, g_ex_vertex_buffer_data, sizeof (GLfloat) * 8);
	memcpy (g_element_buffer_data, g_ex_element_buffer_data, sizeof (GLuint) * 4);
	refresh_buffers ();
	//g_vertex_buffer_fill = 4;
	//g_resources.texture = make_texture ("testure.png");
	
	//Make the texture
	refresh_textures ();
	
	//Make and bind the shaders
	g_resources.vertex_shader = make_shader (GL_VERTEX_SHADER, "vertex_shader.glsl");
	g_resources.frag_shader = make_shader (GL_FRAGMENT_SHADER, "fragment_shader.glsl");
	g_resources.program = make_program (g_resources.vertex_shader, g_resources.frag_shader);
	
	//Find the uniforms
	g_resources.uniforms.texture = glGetUniformLocation (g_resources.program, "texture");
	
	//Find the attributes
	g_resources.attributes.position = glGetAttribLocation (g_resources.program, "position");
	g_resources.attributes.tex = glGetAttribLocation (g_resources.program, "texcoord");
	g_resources.attributes.texid = glGetAttribLocation (g_resources.program, "texid");
	printf ("Attributes handle: %d\n", g_resources.attributes.position);
}

void init () {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_BLEND );
	frame_count = 0;
	g_resources.refresh_texture = 1;
	make_resources ();
}

void render () {
	//default_draw (test_obj);
	refresh_buffers ();
	refresh_textures ();
	glUseProgram (g_resources.program);
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_resources.texture);
    glUniform1i(g_resources.uniforms.texture, 0);
	glBindBuffer(GL_ARRAY_BUFFER, g_resources.vertex_buffer);
	GLuint stride_len = sizeof(GLfloat) * 2;
	GLuint offset = 0;
	//Map the position
    glVertexAttribPointer(
        g_resources.attributes.position,  // attribute
        4,                                // size 
        GL_FLOAT,                         // type 
        GL_FALSE,                         // normalized? 
        stride_len,                       // stride 
        (void*)offset                     // array buffer offset 
    );
	glEnableVertexAttribArray(g_resources.attributes.position);
	//Bind the vertex buffer and render
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_resources.element_buffer);
    glDrawElements(
        GL_TRIANGLE_STRIP,			// mode
        4,							// count
        GL_UNSIGNED_INT,        	// type
        (void*)0            		// element array buffer offset
    );
	glDisableVertexAttribArray(g_resources.attributes.position);
	glutSwapBuffers ();
}

void display () {
	//Do frame logic
	mat4 mat_rotx, mat_roty, mat_rotz;
	matrix_rotx4 (&mat_rotx, camera_ang.x);
	matrix_roty4 (&mat_roty, camera_ang.y);
	matrix_rotz4 (&mat_rotz, camera_ang.z);
	v4 unit, travel_dir, travel_dist, res1, res2;
	initv4 (&unit, 0, 0, 1, 1);
	matrix_mul4v (&res1, &mat_rotz, &unit);
	matrix_mul4v (&res2, &mat_roty, &res1);
	matrix_mul4v (&travel_dir, &mat_rotx, &res2);
	scale3 (&travel_dist, &travel_dir, .1);
	if (get_inputs ()->keys_down ['w']) {
		camera_pos.x += travel_dist.x;
		camera_pos.y += travel_dist.y;
		camera_pos.z += travel_dist.z;
	}
	if (get_inputs ()->keys_down ['a']) {
		camera_ang.z -= 3.14 / 90;
	}
	if (get_inputs ()->keys_down ['s']) {
		camera_pos.x -= travel_dist.x;
		camera_pos.y -= travel_dist.y;
		camera_pos.z -= travel_dist.z;
	}
	if (get_inputs ()->keys_down ['d']) {
		camera_ang.z += 3.14 / 90;
	}
	if (get_inputs ()->mouse_buttons_down[0]) {
		if (!click_point) {
			click_point = newv2 (get_inputs ()->mouse_x, get_inputs ()->mouse_y);
			hrot_base = camera_ang.y;
			vrot_base = camera_ang.x;
		} else {
			double xoffs = get_inputs ()->mouse_x - click_point->x;
			double yoffs = get_inputs ()->mouse_y - click_point->y;
			camera_ang.y = hrot_base + xoffs * 2;
			camera_ang.x = vrot_base + yoffs * 2;
		}
	} else {
		free (click_point);
		click_point = (void*)0;
	}
	if (get_inputs ()->keys_pressed['r']) {
		set_global_args (rargs_render);
		printf ("Rendering scene at resolution %dx%d, with recursive depth %d and sampling rate of %d rays per pixel.\n", rargs_render->img_width, rargs_render->img_height, rargs_render->max_depth, rargs_render->num_samples);
		printf ("Number of render threads: %d\n", rargs_render->num_threads);
		uint8_t* img_buffer = malloc (rargs_render->img_width * rargs_render->img_height * 3 * sizeof (uint32_t));
		#ifdef FIXED_CAM
		render_frame (img_buffer, rargs_render->img_width, rargs_render->img_height, global_cam, global_sc, 1);
		#endif
		#ifndef FIXED_CAM
		render_frame (img_buffer, rargs_render->img_width, rargs_render->img_height, global_cam2, global_sc, 1);
		#endif
		write_img (rargs_render->out_path, img_buffer, rargs_render->img_width, rargs_render->img_height);
		set_global_args (rargs_rt);
	}
	#ifdef FIXED_CAM
	camera_move (global_cam, &camera_pos, &camera_ang);
	tri buf[4];
	camera_get_geom (global_cam, &buf);
	int i;
	for (i = 0; i < 4; i++) {
		camtris [i]->geom = buf[i];
	}
	#endif
	#ifndef FIXED_CAM
	camera_move (global_cam2, &camera_pos, &camera_ang);
	#endif
	//Set timing properties
	frame_start_time = glutGet (GLUT_ELAPSED_TIME);
	frame_count++;
	//Render
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	render_frame ((uint8_t*)render_img_buffer, RT_OUTPUT_WIDTH, RT_OUTPUT_HEIGHT, global_cam2, global_sc, 0);
	refresh_all_textures ();
	render ();
	//Wait for target FPS
	printf ("%d ms\n", glutGet (GLUT_ELAPSED_TIME) - frame_start_time);
	while (glutGet (GLUT_ELAPSED_TIME) - frame_start_time < 17) {}
	swap_input_buffers ();
	glutPostRedisplay ();
}

void test () {

}

int get_frame_count () {
	return frame_count;
}

int get_frame_time_ms () {
	return frame_start_time;
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
	material* glass = material_init_dielectric (malloc (sizeof (material)), 0x00E0E0E0, .99, 5);
	material* mirror = material_init_specular (malloc (sizeof (material)), 0x00E0E0E0, .99, 0);
	sphere_init ((scene_obj_sphere*)scene_alloc_obj (sc), glass, newv3 (0, -.6, 2), .4);
	sphere_init ((scene_obj_sphere*)scene_alloc_obj (sc), red, newv3 (-.6, -.7, 1), .3);
	sphere_init ((scene_obj_sphere*)scene_alloc_obj (sc), blue, newv3 (1.9, 1.1, 4.4), 2);
	sphere_init ((scene_obj_sphere*)scene_alloc_obj (sc), mirror, newv3 (2.3, .2, 1.8), .4);
	sphere_init ((scene_obj_sphere*)scene_alloc_obj (sc), purple, newv3 (-1.8, .8, 2), .6);
	//sphere_init ((scene_obj_sphere*)scene_alloc_obj (sc), white, newv3 (0, -1000, 0), 998);*/
	scene_tri_init ((scene_obj_tri*)scene_alloc_obj (sc), white, &wt);
	#ifdef FIXED_CAM
	camtris [0] = scene_tri_init ((scene_obj_tri*)scene_alloc_obj (sc), white, &wt);
	camtris [1] = scene_tri_init ((scene_obj_tri*)scene_alloc_obj (sc), white, &wt);
	camtris [2] = scene_tri_init ((scene_obj_tri*)scene_alloc_obj (sc), white, &wt);
	camtris [3] = scene_tri_init ((scene_obj_tri*)scene_alloc_obj (sc), white, &wt);
	#endif
	
}

int main (int argc, char** argv) {

	//Load render args
	rargs_rt = malloc (sizeof (render_args));
	rargs_rt->num_threads = 1;
	rargs_rt->img_width = RT_OUTPUT_WIDTH;
	rargs_rt->img_height = RT_OUTPUT_HEIGHT;
	rargs_rt->num_samples = 4;
	rargs_rt->max_depth = 16;
	#ifdef FIXED_CAM
	rargs_rt->fixed_cam = 1;
	#endif
	set_global_args (rargs_rt);
	int OUTPUT_IMG_WIDTH = RT_OUTPUT_WIDTH;
	int OUTPUT_IMG_HEIGHT = RT_OUTPUT_HEIGHT;
	
	rargs_render = malloc (sizeof (render_args));
	get_render_args (rargs_render, argc, argv);
	
	//Init the scene
	global_cam = camera_init (malloc (sizeof (camera)));
	global_cam2 = camera_init (malloc (sizeof (camera)));
	global_sc = scene_init (malloc (sizeof (scene)));
	make_scene (global_sc);
	
	//Render the scene
	#ifdef FIXED_CAM
	initv3 (&camera_pos, 0, 0, 0);
	initv3 (&camera_ang, 0, 0, 0);
	camera_move (global_cam, &camera_pos, &camera_ang);
	v3 cam2pos, cam2rot;
	initv3 (&cam2pos, 0, 0, 0);
	initv3 (&cam2rot, 0, 0, 0);
	camera_move (global_cam2, &cam2pos, &cam2rot);
	#endif
	#ifndef FIXED_CAM
	initv3 (&camera_pos, 0, 0, 0);
	initv3 (&camera_ang, 0, 0, 0);
	camera_move (global_cam, &camera_pos, &camera_ang);
	#endif
	char* img_buffer = malloc (OUTPUT_IMG_WIDTH * OUTPUT_IMG_HEIGHT * 3 * sizeof (uint32_t));
	render_frame (img_buffer, OUTPUT_IMG_WIDTH, OUTPUT_IMG_HEIGHT, global_cam2, global_sc, 0);
	render_img_buffer = img_buffer;
	
	printf ("HELLO WORLD");
	//TEST END
	glutInit (&argc, argv);
	glutInitDisplayMode (GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow ("Hello World");
	glewInit ();
	printf ("OpenGL %s\n", glGetString (GL_VERSION));
	init_inputs ();
	glutDisplayFunc (display);
	glutKeyboardFunc (key_down_handler);
	glutKeyboardUpFunc (key_up_handler);
	glutMouseFunc (mouse_handler);
	glutMotionFunc (mouse_motion_handler);
	glutPassiveMotionFunc (passive_mouse_motion_handler);
	init ();
	test ();
	glutMainLoop ();
	return 0;
	
}