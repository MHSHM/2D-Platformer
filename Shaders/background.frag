#version 330

in vec2 o_uvcoord;
in vec3 o_color; 

uniform sampler2D texture_map;

out vec4 FragColor;

void main()
{
	vec4 color = texture(texture_map, o_uvcoord).rgba;
	FragColor = vec4(color);

}