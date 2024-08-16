#version 330 core

layout(location = 0) in vec4 pos;

uniform mat4 persp;
uniform mat4 view;
uniform mat4 transform;
uniform float time;
uniform float scale;

const float SPEED = 16.0;

out vec2 tc;

void main()
{	
	float id = float(gl_InstanceID);

	vec3 cameraRightWorldSpace = vec3(view[0][0], view[1][0], view[2][0]);
	vec3 cameraUpWorldSpace = vec3(view[0][1], view[1][1], view[2][1]);
	vec4 center = transform * vec4(0.0, 0.0, 0.0, 1.0);
	float maxsz = 16.0 + cos(id) * 6.0;
	float sz = (maxsz - maxsz * pow(1.0 - time, 2.0)) * scale;

	float rotationSpeed = sin(id * cos(id)) * 3.14 / 4.0;
	float rotation = rotationSpeed * time;

	vec2 p = vec2(
		pos.x * cos(rotation) - pos.z * sin(rotation),
		pos.x * sin(rotation) + pos.z * cos(rotation)
	);

	vec3 vertPosWorldSpace =
		center.xyz +
		cameraRightWorldSpace * p.x * sz +
		cameraUpWorldSpace * p.y * sz;

	float angle1 = sin(id * cos(id)) * 2.0 * 3.14;
	float angle2 = cos(sin(id) + cos(id)) * 2.0 * 3.14;
	float d = (fract(id * 31.75414) * 10.0 + 10.0) * scale;
	vertPosWorldSpace += vec3(
		cos(angle1) * cos(angle2), 
		sin(angle1), 
		cos(angle1) * sin(angle2)
	) * d;
	vertPosWorldSpace += vec3(
		cos(angle1) * cos(angle2) * SPEED, 
		SPEED / 2.0,
		cos(angle1) * sin(angle2) * SPEED
	) * time * scale;

	gl_Position = persp * view * vec4(vertPosWorldSpace.xyz, 1.0);

	tc = pos.xz;
	tc += vec2(1.0, 1.0);
	tc /= 2.0;
}
