#version 330

layout (location = 0) in vec2 position;

out vec2 texcoord_final;

void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
	texcoord_final = vec2(position.x * .5 + .5, -position.y * .5 + .5);
}