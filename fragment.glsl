#version 400

in vec3 color_frag;
in vec2 uv_frag;

uniform sampler2D tex;

out vec4 fragment_color;
void main () 
{
	fragment_color = texture(tex, uv_frag);
}