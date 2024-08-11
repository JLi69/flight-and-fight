#version 330 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 norm;

uniform float time;
uniform vec3 velocity;

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
	float t = min(0.003 * gl_InstanceID, time);
	vec4 transformed = pos;
	transformed *= 1.0 / (1.0 + 40.0 * t);
	transformed.w = 1.0;
	transformed = transform * transformed;
	transformed -= t * vec4(velocity.xyz, 0.0);
	gl_Position = persp * view * transformed;
	fragpos = transformed.xyz;
	normal = normalize(normalmat * norm);
	lighting = max(-dot(lightdir, normal), 0.0) * 0.8 + 0.2;
	tc = texcoord;
}
