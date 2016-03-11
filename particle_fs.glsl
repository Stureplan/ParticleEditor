#version 400

in vec2 uv_frag;
in float life_frag;

uniform sampler2D tex;
uniform int glow;
uniform int fade;
uniform vec3 color;

out vec4 fragment_color;
void main () 
{
	fragment_color = texture(tex, uv_frag);
	if (glow == 1)
	{
		fragment_color.xyz = vec3(0.0f, 1.0f, 0.0f);
	}

	//Fade in
	if(fade == -1)
	{
		fragment_color.w *= 1.0f -life_frag;
	}

	//Fade out
	if (fade == 1)
	{
		fragment_color.w *= life_frag;
	}

	fragment_color.xyz *= color;
}