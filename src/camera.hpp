#ifndef __CAMERA_H__
#include <glm/glm.hpp>
#include "geometry.hpp"

enum MovementDirection {
	NONE,
	FORWARD,
	BACKWARD,
	STRAFE_LEFT,
	STRAFE_RIGHT,
	FLY_UP,
	FLY_DOWN
};

struct CameraMovement {
	MovementDirection 
		movement = NONE,
		strafe = NONE,
		flying = NONE; 
	CameraMovement(MovementDirection m, MovementDirection s, MovementDirection f);
};

MovementDirection handlePress(MovementDirection currentDir, MovementDirection d);
MovementDirection handleRelease(MovementDirection currentDir, MovementDirection d);

struct Camera {
	glm::vec3 position = glm::vec3(0.0f);
	float pitch = 0.0f, yaw = 0.0f;

	MovementDirection 
		movementDirection = NONE,
		strafeDirection = NONE,
		flyingDirection = NONE;

	glm::mat4 viewMatrix();
	glm::vec3 forward();
	glm::vec3 right();
	glm::vec3 up();

	Camera();
	Camera(glm::vec3 pos);
	glm::vec3 velocity();
	void fly(float dt, float speed);
	void rotateCamera(float dmousex, float dmousey, float sensitivity);
	void updateMovement(CameraMovement m, bool pressed);
	geo::Frustum getViewFrustum(float znear, float zfar, float aspect, float fovy);
};

#define __CAMERA_H__
#endif
