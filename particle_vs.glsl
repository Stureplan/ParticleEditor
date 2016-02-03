#version 400

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_direction;

out vec3 dir_geom;
			
void main () 
{
	gl_Position = vec4(vertex_position, 1);
	dir_geom = vertex_direction;
}