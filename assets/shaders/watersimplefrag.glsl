#version 330 core

out vec4 color;

in vec3 fragpos;

uniform vec3 lightdir;
uniform vec3 camerapos;

uniform float viewdist;

const float FOG_DIST = 128.0;
const float WATER_FOG_DIST = 24.0;

void main()
{
	float d = length(fragpos - camerapos);
	const vec3 normal = vec3(0.0, 1.0, 0.0);
	//specular reflection
	vec3 reflected = reflect(lightdir, normal);
	vec3 viewdir = normalize(camerapos - fragpos);
	float specular = pow(max(dot(viewdir, reflected), 0.0), 8.0);
	//diffuse lighting
	float diffuse = max(-dot(lightdir, normal), 0.0) * 0.5 + 0.5;

	color = 
		vec4(0.1, 0.7, 0.9, 0.0) * diffuse + 
		vec4(0.5, 0.3, 0.1, 0.0) * specular;
	color.a = 0.8;

	//fog
	vec4 fogeffect = mix(color, vec4(0.5, 0.8, 1.0, 1.0), min(max(0.0, d - viewdist) / FOG_DIST, 1.0));
	vec4 watereffect = mix(color, vec4(0.1, 0.7, 0.9, 1.0), min(max(0.0, d) / WATER_FOG_DIST, 1.0));
	color = fogeffect * float(camerapos.y >= 0.0) + watereffect * float(camerapos.y < 0.0);
}
