#version 330 core

layout (std140) uniform GlobalVals {
	float viewdist;
};

out vec4 color;

in vec3 fragpos;

//we combine the normal maps and the dudv map into one texture for
//optimization purposes - I'm not entirely sure why this works but it does
//seem to be faster and from my reading seems to do with the fact that texture
//lookups can be somewhat expensive due to needing to load a large amount of
//data into memory so to play nice with the cache I've combined the textures
uniform sampler2D watermaps;

uniform float time;
uniform vec3 lightdir;
uniform vec3 camerapos;

const float FOG_DIST = 10000.0;
const float WATER_FOG_DIST = 128.0;

const float angle1 = -0.5;
const float angle2 = 3.14 / 2.0 - 0.25;
const vec2 dir1 = vec2(cos(angle1), sin(angle1));
const vec2 dir2 = vec2(cos(angle2), sin(angle2));

void main()
{
	vec2 dudv = (texture(watermaps, fract(fragpos.xz / 64.0) * vec2(0.25, 1.0) + vec2(0.5, 0.0)).xy - 0.5) * 2.0;
	vec2 tc1 = fract((fragpos.xz + dudv) / 64.0 + dir1 * 0.08 * time) * vec2(0.25, 1.0);
	//This clamp minimizes the visibility of a seam in the water but it's still
	//visible if you look hard and close enough. I don't really have any other
	//better ways of handling this
	tc1.x = clamp(tc1.x, 0.005, 0.2495);
	vec2 tc2 = fract((fragpos.xz + dudv) / 96.0 + dir2 * 0.05 * time) * vec2(0.25, 1.0) + vec2(0.25, 0.0);
	tc2.x = clamp(tc2.x, 0.2505, 0.4995);
	vec3 n1 = (texture(watermaps, tc1).xzy - 0.5) * 2.0,
		 n2 = (texture(watermaps, tc2).xzy - 0.5) * 2.0;
	vec3 normal = normalize(n1 + n2);
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
	float d = length(fragpos - camerapos);
	vec4 fogeffect = mix(color, vec4(0.5, 0.8, 1.0, 1.0), clamp((d - viewdist) / FOG_DIST, 0.0, 1.0));
	vec4 watereffect = mix(color, vec4(0.1, 0.7, 0.9, 1.0), clamp(d / WATER_FOG_DIST, 0.0, 1.0));
	color = fogeffect * float(camerapos.y >= 0.0) + watereffect * float(camerapos.y < 0.0);
}
