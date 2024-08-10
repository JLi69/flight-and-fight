#version 330 core

layout(location = 0) in vec4 pos;

uniform mat4 screen;
uniform mat4 transform;

out vec2 tc;
out vec3 fragpos;
out vec2 pos2d;

void main()
{
	vec4 p = transform * pos;
	gl_Position = screen * p;
	fragpos = p.xyz;
	tc = pos.xz;
	tc += vec2(1.0, 1.0);
	tc /= 2.0;
	pos2d = pos.xz;
}
