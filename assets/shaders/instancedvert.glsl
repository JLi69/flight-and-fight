#version 330 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 norm;

uniform int range;
uniform float scale;

uniform mat4 persp;
uniform mat4 view;
uniform mat4 transform;

uniform vec3 lightdir;
out float lighting;

out vec3 fragpos;

void main()
{
	float
		x = float(gl_InstanceID % (2 * range + 1) - range) * scale * 2.0,
		z = float(int(gl_InstanceID / (2 * range + 1)) - range) * scale * 2.0;
	gl_Position = persp * view * (transform * pos + vec4(x, 0.0, z, 0.0));
	fragpos = (transform * pos).xyz + vec3(x, 0.0, z);
	lighting = max(-dot(lightdir, norm), 0.0) * 0.5 + 0.5;
}
