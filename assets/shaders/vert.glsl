#version 330 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 norm;

uniform mat4 persp;
uniform mat4 view;
uniform mat4 transform;
uniform mat3 normalmat;

uniform vec3 lightdir;
out float lighting;

out vec3 fragpos;
out vec2 tc;
out vec3 normal;

void main()
{
	gl_Position = persp * view * transform * pos;
	fragpos = (transform * pos).xyz;
	normal = normalize(normalmat * norm);
	lighting = max(-dot(lightdir, normal), 0.0) * 0.8 + 0.2;
	tc = texcoord;
}
