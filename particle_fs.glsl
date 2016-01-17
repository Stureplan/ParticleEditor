#version 400

in vec2 uv_frag;

uniform sampler2D tex;

out vec4 fragment_color;
void main () 
{
	fragment_color = texture(tex, uv_frag);
	//fragment_color = vec4(1.0f, 1.0f, 0.0f, 1.0f);
}