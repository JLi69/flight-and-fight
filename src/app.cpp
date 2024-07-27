#include "app.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <stb_image/stb_image.h>

State::State() 
{
	cam = Camera(glm::vec3(0.0f, 360.0f, 0.0f));
	mousex = 0.0;
	mousey = 0.0;
	persp = glm::mat4(1.0f);

	currentFovy = 0.0f;
	currentAspect = 0.0f;
	currentZnear = 0.0f;
	currentZfar = 0.0f;

	window = nullptr;
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

void State::updatePerspectiveMat(float fovy, float znear, float zfar)
{
	int w, h;
	glfwGetWindowSize(window, &w, &h);
	const float aspect = float(w) / float(h);
	persp = glm::perspective(fovy, aspect, znear, zfar);	

	currentFovy = fovy;
	currentAspect = aspect;
	currentZnear = znear;
	currentZfar = zfar;
}

float State::getFovy()
{
	return currentFovy;
}

float State::getAspect()
{
	return currentAspect;
}

float State::getZnear()
{
	return currentZnear;
}

float State::getZfar()
{
	return currentZfar;
}

void State::setKey(int key, KeyState keystate)
{
	keystates[key] = keystate;
}

void State::updateKeyStates()
{
	for(auto &keystate : keystates)
		if(keystate.second == JUST_PRESSED)
			keystate.second = HELD;
}

KeyState State::getKeyState(int key)
{
	return keystates[key];
}

glm::mat4 State::getPerspective()
{
	return persp;
}

GLFWwindow* State::getWindow()
{
	return window;
}

void State::createWindow(const char *name, int w, int h)
{
	window = glfwCreateWindow(w, h, name, nullptr, nullptr);
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
	state->setMousePos(x, y);
}

void handleKeyInput(GLFWwindow *window, int key, int scancode, int action, int mods)
{	
	//Toggle cursor
	if(key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
		int cursorMode = glfwGetInputMode(window, GLFW_CURSOR);
		glfwSetInputMode(
			window,
			GLFW_CURSOR,
			cursorMode == GLFW_CURSOR_DISABLED ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED
		);
	}

	if(action == GLFW_PRESS)
		State::get()->setKey(key, JUST_PRESSED);
	else if(action == GLFW_RELEASE)
		State::get()->setKey(key, RELEASED);
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

void initWindow(GLFWwindow* window)
{
	if(!window)
		die("Failed to create window!");
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glfwSetWindowSizeCallback(window, handleWindowResize);
	glfwSetKeyCallback(window, handleKeyInput);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//Set icon of window
	int channels;
	GLFWimage icon[1];
	icon->pixels = nullptr;
	icon->pixels = stbi_load("assets/icon.png", &icon->width, &icon->height, &channels, 0);
	if(!icon->pixels)
		die("Failed to load icon.png!");
	glfwSetWindowIcon(window, 1, icon);
	stbi_image_free(icon->pixels); //Free icon data

	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		die("Failed to init glad!");
	initMousePos(window);
}

bool keyIsHeld(KeyState keystate)
{
	return keystate == JUST_PRESSED || keystate == HELD;
}
