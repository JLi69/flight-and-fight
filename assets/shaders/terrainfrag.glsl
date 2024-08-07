#version 330 core

layout (std140) uniform GlobalVals {
	float viewdist;
};

out vec4 color;

in float lighting;
in float height;
in vec3 fragpos;

uniform float time;
uniform vec3 lightdir;
uniform vec3 camerapos;

uniform sampler2D terraintexture;

const float FOG_DIST = 10000.0;
const float WATER_FOG_DIST = 128.0;

uniform vec2 center;
uniform float minrange;
uniform float maxrange;
uniform vec3 testcolor;

vec2 getuv1()
{
	return
		vec2(0.0, 0.0) * float(height < 0.01) +
		vec2(0.25, 0.0) * float(height >= 0.01 && height < 0.1) +
		vec2(0.50, 0.0) * float(height >= 0.1 && height < 0.6) +
		vec2(0.75, 0.0) * float(height >= 0.6);
}

vec2 getuv2()
{
	return
		vec2(0.0, 0.0) * float(height < 0.02) +
		vec2(0.25, 0.0) * float(height >= 0.02 && height < 0.3) +
		vec2(0.50, 0.0) * float(height >= 0.3 && height < 0.7) +
		vec2(0.75, 0.0) * float(height >= 0.7);
}

float mixval(float lower, float upper, float y)
{
	return (1.0 - (y - lower) / (upper - lower)) * float(y >= lower && y < upper);
}

float heightToMix()
{
	return
		mixval(0.01, 0.02, height) +
		mixval(0.1, 0.3, height) +
		mixval(0.6, 0.7, height);
}

vec4 getcolor()
{
	vec2 tc = fract(fragpos.xz / 16.0) * vec2(0.248, 1.0) + vec2(0.001, 0.0);
	vec2 uv1 = getuv1();
	vec2 uv2 = getuv2();

	vec4 color1 = texture(terraintexture, tc + uv1);
	vec4 color2 = texture(terraintexture, tc + uv2);
	color1.a = 0.0;
	color2.a = 0.0;

	return mix(color1, color2, heightToMix());
}

void main()
{
	float d = length(fragpos - camerapos);

	float range = max(abs(fragpos.x - center.x), abs(fragpos.z - center.y));
	//We discard fragments that are beyond a certain range to prevent overlap
	//with terrain of lower level of detail
	if((range > maxrange && maxrange > 0.0) || range < minrange)
		discard;

	color = getcolor() * lighting;
	//uncomment this line whenever you want to display the different levels of detail
	//color = vec4(testcolor, 1.0) * lighting;
	color.a = 1.0;
	//fog
	vec4 fogeffect = mix(color, vec4(0.5, 0.8, 1.0, 1.0), min(max(0.0, d - viewdist) / FOG_DIST, 1.0));
	vec4 watereffect = mix(color, vec4(0.1, 0.7, 0.9, 1.0), min(max(0.0, d) / WATER_FOG_DIST, 1.0));
	color = fogeffect * float(camerapos.y >= 0.0) + watereffect * float(camerapos.y < 0.0);
}
