#version 330

uniform sampler2D texture;

in vec2 texcoord_final;

void main()
{
    vec4 c = texture2D (texture, texcoord_final);
    gl_FragColor = c;
}