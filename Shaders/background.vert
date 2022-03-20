#version 330 core 

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uvcoord;

uniform vec3 in_color; 

uniform mat4 in_world;
uniform mat4 in_orthoproj; 

out vec2 o_uvcoord;

void main()
{
	gl_Position = in_orthoproj * in_world * vec4(in_position, 1.0f);
	o_uvcoord = in_uvcoord; 
}