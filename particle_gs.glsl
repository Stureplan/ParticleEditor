#version 400

layout (points) in;
layout (triangle_strip, max_vertices=4) out;

in vec3 dir_geom[];
in float life_geom[];

uniform mat4 MVP;
uniform vec3 cam;
uniform vec2 size;
uniform int scaledir;

out vec2 uv_frag;
out float life_frag;

void main()
{
	vec3 pos = gl_in[0].gl_Position.xyz;
	vec3 cam_normal = normalize(cam - pos);
	//vec3 up = vec3(0.0f, 1.0f, 0.0f);
	vec3 up = dir_geom[0];
	vec3 right = cross(cam_normal, up);

	up = up * size.y;
	right = right * size.x;

	if (scaledir == -1)
	{
		//Scale down
		up *= life_geom[0];
		right *= life_geom[0];
	}

	if (scaledir == 0)
	{
		//Do nothing, leave scale as is
	}

	if (scaledir == 1)
	{
		//Scale up
		up *= 1.0f - life_geom[0];
		right *= 1.0f - life_geom[0];
	}


	life_frag = life_geom[0];
	vec3 cPos = pos;

	//VTX 1
	cPos = pos - right - up;
	gl_Position = MVP * vec4(cPos, 1.0f);
	uv_frag = vec2(0.0f, 0.0f);
	EmitVertex();

	//VTX 2
	cPos = pos + right - up;
	gl_Position = MVP * vec4(cPos, 1.0f);
	uv_frag = vec2(1.0f, 0.0f);
	EmitVertex();

	//VTX 3
	cPos = pos + up - right;
	gl_Position = MVP * vec4(cPos, 1.0f);
	uv_frag = vec2(0.0f, 1.0f);
	EmitVertex();

	//VTX 4
	cPos = pos + up + right;
	gl_Position = MVP * vec4(cPos, 1.0f);
	uv_frag = vec2(1.0f, 1.0f);
	EmitVertex();
	
	EndPrimitive();
}