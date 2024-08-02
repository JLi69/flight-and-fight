#version 330 core

uniform sampler2D tex;
uniform float time;

in vec2 tc;

out vec4 color;

void main()
{
	color = texture(tex, tc);
	color *= mix(
		vec4(1.0, 1.0, 1.0, 1.0), 
		vec4(0.1, 0.1, 0.1, 1.0),
		min(pow(time / 1.5, 3.0), 1.0)
	);
}
