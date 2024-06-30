#version 330 core

layout(location = 0) in float y;
layout(location = 1) in vec2 norm;

uniform mat4 persp;
uniform mat4 view;
uniform mat4 transform;

uniform vec3 lightdir;
uniform float maxheight;
uniform float chunksz;
uniform int prec;

out float lighting;
out float height;
out vec3 fragpos;

void main()
{
	int ix = gl_VertexID - int(gl_VertexID / (prec + 1)) * (prec + 1);
	int iz = int(gl_VertexID / (prec + 1));

	float halfinc = chunksz / float(prec + 1);
	float vx = -chunksz + float(ix) / float(prec + 1) * 2.0 * chunksz + halfinc;
	float vz = -chunksz + float(iz) / float(prec + 1) * 2.0 * chunksz + halfinc;
	vec4 pos = vec4(vx, y * maxheight, vz, 1.0);
	height = pos.y / maxheight;
	gl_Position = persp * view * transform * pos;
	fragpos = (transform * pos).xyz;

	vec3 normal = vec3(cos(norm.y) * cos(norm.x), sin(norm.y), cos(norm.y) * sin(norm.x));
	lighting = max(-dot(lightdir, normal), 0.0) * 0.6 + 0.4;
}
