#version 400

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_color;
layout(location = 2) in vec2 vertex_uv;
		
out vec3 color_frag;
out vec2 uv_frag;

uniform mat4 MVP;
			
void main () 
{
	gl_Position = MVP * vec4(vertex_position, 1);
	color_frag = vertex_color;
	uv_frag = vertex_uv;
}