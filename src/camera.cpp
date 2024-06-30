#define _USE_MATH_DEFINES
#include <math.h>
#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

CameraMovement::CameraMovement(
	MovementDirection m,
	MovementDirection s,
	MovementDirection f
) {
	movement = m;
	strafe = s;
	flying = f;
}

Camera::Camera()
{
	position = glm::vec3(0.0f);
}

Camera::Camera(glm::vec3 pos)
{
	position = pos;
}

glm::vec3 Camera::velocity()
{
	glm::vec3 velocity = glm::vec3(0.0f);
	//Move forwards/backwards
	switch(movementDirection)
	{
	case FORWARD:
		velocity += glm::vec3(sinf(yaw), 0.0f, -cosf(yaw));
		break;
	case BACKWARD:
		velocity -= glm::vec3(sinf(yaw), 0.0f, -cosf(yaw));
		break;
	default:
		break;
	}

	//Strafing
	switch(strafeDirection)
	{
	case STRAFE_LEFT:
		velocity += glm::vec3(sinf(-yaw - M_PI / 2.0f), 
							  0.0f,
							  cosf(-yaw - M_PI / 2.0f));
		break;
	case STRAFE_RIGHT:
		velocity += glm::vec3(sinf(-yaw + M_PI / 2.0f), 
							  0.0f,
							  cosf(-yaw + M_PI / 2.0f));
		break;
	default:
		break;
	}

	if(glm::length(velocity) == 0.0f)
		return glm::vec3(0.0f);

	return glm::normalize(velocity);
}

void Camera::fly(float dt, float speed)
{
	//Fly
	switch(flyingDirection)
	{
	case FLY_UP:
		position += glm::vec3(0.0f, speed, 0.0f) * dt;
		break;
	case FLY_DOWN:
		position -= glm::vec3(0.0f, speed, 0.0f) * dt;
		break;
	default:
		break;
	}
}

//Returns the view matrix
glm::mat4 Camera::viewMatrix()
{
	return glm::rotate(glm::mat4(1.0f), pitch, glm::vec3(1.0f, 0.0f, 0.0f)) *
		   glm::rotate(glm::mat4(1.0f), yaw, glm::vec3(0.0f, 1.0f, 0.0f)) *
		   glm::translate(glm::mat4(1.0f), -position);
}

glm::vec3 Camera::forward()
{
	return glm::normalize(
		glm::vec3(
			cosf(pitch) * sinf(yaw),
			sinf(-pitch),
			cosf(pitch) * -cosf(yaw)
		)
	);
}

glm::vec3 Camera::right()
{
	return glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), forward()));
}

glm::vec3 Camera::up()
{	
	return glm::normalize(glm::cross(forward(), right()));
}

void Camera::rotateCamera(float dmousex, float dmousey, float sensitivity)
{
	//Rotate the yaw of the camera when the cursor moves left and right
	if(dmousex != 0.0f)
		yaw += 0.05f * sensitivity * dmousex;

	//Rotate the pitch of the camera when the cursor moves up and down
	if(dmousey != 0.0f)
		pitch += 0.05f * sensitivity * dmousey; 

	//Clamp the pitch to be between -pi / 2 radians and pi / 2 radians
	if(pitch < -M_PI / 2.0f)
		pitch = -M_PI / 2.0f;
	if(pitch > M_PI / 2.0f)
		pitch = M_PI / 2.0f;
}


MovementDirection handlePress(MovementDirection currentDir, MovementDirection d)
{
	if(d != NONE)
		return d;
	return currentDir;
}

MovementDirection handleRelease(MovementDirection currentDir, MovementDirection d)
{
	if(d == currentDir)
		return NONE;
	return currentDir;
}

void Camera::updateMovement(CameraMovement m, bool pressed)
{
	if(pressed) {
		movementDirection = handlePress(movementDirection, m.movement);
		strafeDirection = handlePress(strafeDirection, m.strafe);
		flyingDirection = handlePress(flyingDirection, m.flying);
	}
	else {
		movementDirection = handleRelease(movementDirection, m.movement);
		strafeDirection = handleRelease(strafeDirection, m.strafe);
		flyingDirection = handleRelease(flyingDirection, m.flying);
	}
}

geo::Frustum Camera::getViewFrustum(float znear, float zfar, float aspect, float fovy)
{
	float halfHeight = zfar * tanf(fovy / 2.0f);
	float halfWidth = halfHeight * aspect;
	return {
		.back = geo::Plane(position + znear * forward(), forward()),
		.front = geo::Plane(position + zfar * forward(), -forward()),
		.top = geo::Plane(position, glm::cross(right(), zfar * forward() + up() * halfHeight)),
		.bottom = geo::Plane(position, glm::cross(zfar * forward() - up() * halfHeight, right())),
		.left = geo::Plane(position, glm::cross(up(), zfar * forward() - right() * halfWidth)),
		.right = geo::Plane(position, glm::cross(zfar * forward() + right() * halfWidth, up())),
	};
}
