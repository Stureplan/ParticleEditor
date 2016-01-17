#version 400

layout (lines) in;
layout (line_strip, max_vertices=2) out;

in vec3[] color_geom;
in vec2[] uv_geom;

out vec3 color_frag;
out vec2 uv_frag;

void main()
{
	for (int i = 0; i < 2; i++)
	{
		gl_Position = gl_in[i].gl_Position;
		color_frag = color_geom[i];
		uv_frag = uv_geom[i];

		EmitVertex();
	}

	
	EndPrimitive();
}