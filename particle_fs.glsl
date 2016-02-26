#version 400

in vec2 uv_frag;

uniform sampler2D tex;
uniform int glow;

out vec4 fragment_color;
void main () 
{
	fragment_color = texture(tex, uv_frag);
	if (glow == 1)
	{
		fragment_color.xyz = vec3(0.0f, 1.0f, 0.0f);
	}
}