#version 330 core

in vec2 pos2d;
in vec3 fragpos;

out vec4 color;

void main()
{
	float a = step(length(pos2d), 1.0);
	float green = float(
		(fract(pos2d.x * 4.0) < 0.05 || 
		fract(pos2d.x * 4.0) > 0.95) ||
		(fract(pos2d.y * 4.0) < 0.05 || 
		fract(pos2d.y * 4.0) > 0.95) ||
		fract(length(pos2d) * 3.1) < 0.1
	);
	color = vec4(0.0, 0.0, 0.0, a) * (1.0 - green) + vec4(0.0, 0.5, 0.0, a) * green;
}
