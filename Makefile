render: render.c vertex vector matrix tri scene camera material rng render_args
	gcc -g render.c vertex.o vector.o tri.o scene.o camera.o material.o rng.o render_args.o matrix.o -lm -lpthread -Wall -o a.exe
vertex: vertex.c vertex.h
	gcc -c -g vertex.c -Wall
vector: vector.c vector.h
	gcc -c -g vector.c -Wall
matrix: matrix.c matrix.h
	gcc -c -g matrix.c -Wall
tri: tri.c tri.h
	gcc -c -g tri.c
scene: scene.c scene.h
	gcc -c -g scene.c -Wall
camera: camera.c camera.h
	gcc -c -g camera.c -Wall
material: material.c material.h
	gcc -c -g material.c -Wall
rng: rng.c rng.h
	gcc -c -g rng.c
render_args: render_args.c render_args.h
	gcc -c -g render_args.c