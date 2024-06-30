#version 330 core

in vec3 fragpos;
out vec4 color;

uniform samplerCube skybox;

void main()
{
	float v = clamp((0.4 - fragpos.y) * 4.0, 0.0, 1.0);
	color = mix(texture(skybox, fragpos), vec4(0.5, 0.8, 1.0, 1.0), v);
}
