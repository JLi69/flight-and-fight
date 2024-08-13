#version 330 core

layout (std140) uniform GlobalVals {
	float viewdist;
};

uniform sampler2D tex;

out vec4 color;
in vec2 tc;
in float lighting;
in vec3 normal;

in vec3 fragpos;

uniform vec3 camerapos;
uniform vec3 lightdir;
//How strong the specular effect is
uniform float specularfactor;

const float FOG_DIST = 10000.0;
const float WATER_FOG_DIST = 128.0;

void main()
{
	float d = length(fragpos - camerapos);

	color = texture(tex, fract(tc));

	//We assume that there are not any transparent texels in the texture
	//other than texels with alpha value of 0 and just discard any texel
	//with alpha value that is not 1.0
	if(color.a < 1.0)
		discard;

	//Diffuse lighting
	color *= lighting;
	color.a = 1.0;

	//Specular lighting
	vec3 reflected = normalize(reflect(lightdir, normal));
	float spec = pow(dot(reflected, normalize(camerapos - fragpos)), 16.0) * specularfactor;
	color += vec4(1.0, 1.0, 1.0, 0.0) * spec;
	color = clamp(color, 0.0, 1.0);

	//fog
	vec4 fogeffect = mix(color, vec4(0.5, 0.8, 1.0, 1.0), min(max(0.0, d - viewdist) / FOG_DIST, 1.0));
	vec4 watereffect = mix(color, vec4(0.1, 0.7, 0.9, 1.0), min(max(0.0, d) / WATER_FOG_DIST, 1.0));
	color = fogeffect * float(camerapos.y >= 0.0) + watereffect * float(camerapos.y < 0.0);
}
