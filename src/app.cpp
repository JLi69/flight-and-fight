#include "app.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <glm/gtc/matrix_transform.hpp>
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

	scrollspeed = 0.0f;

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

double State::getMouseDX()
{
	return mousedx;
}

double State::getMouseDY()
{
	return mousedy;
}

double State::getScrollSpeed()
{
	return scrollspeed;
}

void State::setMousePos(double x, double y)
{
	mousex = x;
	mousey = y;
}

void State::setMouseDiff(double dx, double dy)
{
	mousedx = dx;
	mousedy = dy;
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

void State::setButton(int button, KeyState buttonstate)
{
	mousebuttonstates[button] = buttonstate;
}

void State::setScrollSpeed(double yoff)
{
	scrollspeed = yoff;
}

void State::updateKeyStates()
{
	for(auto &keystate : keystates)
		if(keystate.second == JUST_PRESSED)
			keystate.second = HELD;

	for(auto &buttonstate : mousebuttonstates)
		if(buttonstate.second == JUST_PRESSED)
			buttonstate.second = HELD;

	scrollspeed = 0.0;
	
	mousedx = 0.0;
	mousedy = 0.0;
}

void State::clearInputState()
{
	keystates = {};
	mousebuttonstates = {};
}

KeyState State::getKeyState(int key)
{
	return keystates[key];
}

KeyState State::getButtonState(int button)
{
	return mousebuttonstates[button];
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

void State::initNuklear()
{
	//Init nuklear	
	ctx = nk_glfw3_init(&glfw, getWindow(), NK_GLFW3_INSTALL_CALLBACKS);
}

nk_glfw* State::getNkGlfw()
{
	return &glfw;
}

nk_context* State::getNkContext()
{
	return ctx;
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
	state->setMouseDiff(dx, dy);
}

void handleKeyInput(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(action == GLFW_PRESS)
		State::get()->setKey(key, JUST_PRESSED);
	else if(action == GLFW_RELEASE)
		State::get()->setKey(key, RELEASED);
}

void handleMouseInput(GLFWwindow *window, int button, int action, int mods)
{
	//Nuklear
	nk_glfw3_mouse_button_callback(window, button, action, mods);

	if(action == GLFW_PRESS)
		State::get()->setButton(button, JUST_PRESSED);
	else if(action == GLFW_RELEASE)
		State::get()->setButton(button, RELEASED);
}

void scrollCallback(GLFWwindow *window, double xoff, double yoff)
{
	nk_glfw3_scroll_callback(window, xoff, yoff);
	State::get()->setScrollSpeed(yoff);
}

void initMousePos(GLFWwindow *window)
{
	double mousex, mousey;
	glfwGetCursorPos(window, &mousex, &mousey);
	State::get()->setMousePos(mousex, mousey);
}

unsigned int outputFps(float dt, unsigned int &chunksPerSecond)
{
	static unsigned int fps = 0;
	static float fpstimer = 0.0f;
	static int frames = 0;
	fpstimer += dt;

	if(fpstimer > 1.0f) {
		fprintf(stderr, "FPS: %d | Chunks drawn: %d\n", frames, chunksPerSecond);
		fps = frames;
		fpstimer = 0;
		frames = 0;
		chunksPerSecond = 0;
		return fps;
	}
		
	frames++;

	return fps;
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
	glfwSetMouseButtonCallback(window, handleMouseInput);
	glfwSetScrollCallback(window, scrollCallback);

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
