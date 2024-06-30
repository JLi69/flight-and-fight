#include "app.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <map>

State::State() 
{
	cam = Camera(glm::vec3(0.0f, 360.0f, 0.0f));
	mousex = 0.0;
	mousey = 0.0;
}

State* State::get()
{
	static State* state = new State;
	return state;
}

double State::getMouseX()
{
	return mousex;
}

double State::getMouseY() 
{
	return mousey;
}

void State::setMousePos(double x, double y)
{
	mousex = x;
	mousey = y;
}

Camera& State::getCamera()
{
	return cam;
}

void die(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

void handleWindowResize(GLFWwindow *window, int w, int h)
{
	glViewport(0, 0, w, h);
}

void cursorPosCallback(GLFWwindow *window, double x, double y)
{
	State* state = State::get();
	int cursorMode = glfwGetInputMode(window, GLFW_CURSOR);
	double 
		dx = x - state->getMouseX(),
		dy = y - state->getMouseY();
	if(cursorMode == GLFW_CURSOR_DISABLED)
		state->getCamera().rotateCamera(dx, dy, 0.02f);
	state->setMousePos(x, y);
}

void handleKeyInput(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	const std::map<int, CameraMovement> keyToMovement = {
		{ GLFW_KEY_W, CameraMovement(FORWARD, NONE, NONE) },
		{ GLFW_KEY_S, CameraMovement(BACKWARD, NONE, NONE) },
		{ GLFW_KEY_A, CameraMovement(NONE, STRAFE_LEFT, NONE) },
		{ GLFW_KEY_D, CameraMovement(NONE, STRAFE_RIGHT, NONE) },
		{ GLFW_KEY_SPACE, CameraMovement(NONE, NONE, FLY_UP) },
		{ GLFW_KEY_LEFT_SHIFT, CameraMovement(NONE, NONE, FLY_DOWN) },
	};

	//Toggle cursor
	if(key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
		int cursorMode = glfwGetInputMode(window, GLFW_CURSOR);
		glfwSetInputMode(
			window,
			GLFW_CURSOR,
			cursorMode == GLFW_CURSOR_DISABLED ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED
		);
	}

	Camera& cam = State::get()->getCamera();
	if(action == GLFW_PRESS && keyToMovement.count(key))
		cam.updateMovement(keyToMovement.at(key), true);
	else if(action == GLFW_RELEASE && keyToMovement.count(key))
		cam.updateMovement(keyToMovement.at(key), false);	
}

void initMousePos(GLFWwindow *window)
{
	double mousex, mousey;
	glfwGetCursorPos(window, &mousex, &mousey);
	State::get()->setMousePos(mousex, mousey);
}

bool outputFps(float dt, unsigned int &chunksPerSecond)
{
	static float fpstimer = 0.0f;
	static int frames = 0;
	fpstimer += dt;

	if(fpstimer > 1.0f) {
		fprintf(stderr, "FPS: %d | Chunks drawn: %d\n", frames, chunksPerSecond);
		fpstimer = 0;
		frames = 0;
		chunksPerSecond = 0;
		return true;
	}
		
	frames++;

	return false;
}
