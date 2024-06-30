#version 330 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 norm;

uniform mat4 persp;
uniform mat4 view;
uniform mat4 transform;

uniform vec3 lightdir;
out float lighting;

out vec3 fragpos;

void main()
{
	gl_Position = persp * view * transform * pos;
	fragpos = (transform * pos).xyz;
	lighting = max(-dot(lightdir, norm), 0.0) * 0.5 + 0.5;
}
