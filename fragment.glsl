#version 400
in vec3 color;
in vec2 uv;

uniform sampler2D tex;

out vec4 fragment_color;
void main () 
{
	fragment_color = texture(tex, uv);
	
}