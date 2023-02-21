This is a raytracer that I implemented as a homework assignment for one of my classes. However, I added a few features that were beyond the scope of the assignment (such as multithreaded rendering and a real-time UI).  
The original repo is also on my github, but it's a private repository because it contains details about the specific class this assignment was for (and I'd rather not have any students in that class copying my solution).  
  
---
  
To build this project, use the included GNU makefile.  
  
---
  
To create scene objects, modify the make_scene function in render.c.  
  
  
To create a material, use material_init, material_init_specular, or material_init_dielectric. They all return a pointer to a material struct.  
Note: Emissive materials are implemented, but they're a little broken and I haven't included documentation on them here. You can try to mess with them if you want though.
  
Signature for material_init: ```material* material_init (void* loc, uint32_t color, double falloff, void (*bounce_ray) (collision_info*, collision_info*, void*, uint64_t*))```  
	- loc is the pointer to initialize the material at.  
	- color is the color of the material, in ARGB format (alpha channel is unused though, so it's just RGB on the low 24 bits)  
	- falloff is a constant that controls the material's color contribution to rays that collide with it. I use .3 to .7 for diffuse materials, and .95 to 1.0 for specular materials.  
	- bounce_ray is the ray bouncing function. Three ray bounce functions are defined in material.h: bounce_none, bounce_diffuse, and bounce_specular.  
	  
Signature for material_init_specular: ```material* material_init_specular (void* loc, uint32_t color, double falloff, double fuzz)```  
	- loc is the same as material_init.  
	- color is the same as material_init.  
	- falloff is the same as material_init.  
	- fuzz is the "fuzz" factor of the specular material.  
  
Signature for material_init_dielectric: ```material* material_init_dielectric (void* loc, uint32_t color, double falloff, double refractive_index)```  
	- loc is the same as material_init.  
	- color is the same as material_init.  
	- falloff is the same as material_init.  
	- refractive_index is the refractive index of the material.  
  
To create a sphere, use sphere_init. It returns a pointer to a scene_obj_sphere struct.  
  
Signature for sphere_init: ```scene_obj_sphere* sphere_init (void* loc, material* mat, v3* pos, double radius)```  
	- loc is the pointer to initialize the sphere at.  
	- mat is the material to use.  
	- pos is the location of the center of the sphere.  
	- radius is the radius of the sphere.  
  
  
To create a triangle, use scene_tri_init. It returns a pointer to a scene_obj_tri struct.  
  
Signature for scene_tri_init: ```scene_obj_tri* scene_tri_init (void* loc, material* mat, tri* geom)```  
	- loc is the pointer to initialize the triangle at.  
	- mat is the material to use.  
	- geom is a pointer to the tri struct containing the verticies to use.  
To make a tri struct:  
	create 3 verticies (v3) using either initv3 or newv3. Pass them to newtri, which returns a tri*.  
Signature for newtri: ```tri* newtri (v3* a, v3* b, v3* c)```  
Signature for initv3: ```v3* initv3 (void* loc, double x, double y, double z)```  
Signature for newv3: ```v3* newv3 (double x, double y, double z)```  
  
  
For moving the camera, modify the function setup_camera in render.c.  
  
The function for moving the camera is camera_move. Its signature is ```void camera_move (camera* cam, v3* position, v3* rotation);```  
  
---
  
I've also included an interactive version, which can be built by building Makefile_interactive with GNUMake (Windows only, must be built with MinGW).
The interactive version has additional dependencies. These are:  
	- opengl32  
	- glew32  
	- glu32  
	- freeglut  
	  
The controls for the interactive version are as follows:  
	- WS to move forwards and backwards  
	- AD to roll  
	- drag the mouse to look around.  
	- After getting the desired perspective, you can press R to render that view at full quality to output.ppm.  
	  
Command line arguments can also be used to configure the output render on the interactive version.  
  
---
  
The executable has several command-line arguments to adjust certain parameters. Alternatively, you can change the default parameters in render_args.h.  
  
-w# specifies the width of the output image in px  
-h# specifies the height of the output image in px  
-s# specifies the number of samples/rays per px  
-d# specifies the maximum recursive depth  
-t# specifies the number of render threads to use. If the number is omitted, it will use the line width as the number of render threads.  
-o soecifies the path of the output file.  
  
For example: ./a.exe -w256 -h144 -s4096 -d64 -t "-oimg.ppm" will render a 256x144px image with a sample rate of 4096 rays per pixel, at a max recursive depth of 64 and using 256 threads. It will output the image to img.ppm.  
(This command is for windows, I'm not sure about the quotation marks around -oimg.ppm in linux environments).  