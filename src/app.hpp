#include <GLFW/glfw3.h>
#include "camera.hpp"
#pragma once

class State {
	Camera cam;
	double mousex, mousey;
	State();
public:
	static State* get();
	double getMouseX();
	double getMouseY();
	void setMousePos(double x, double y);
	Camera& getCamera();
};

void die(const char *msg);
void handleWindowResize(GLFWwindow *window, int w, int h);
void cursorPosCallback(GLFWwindow *window, double x, double y);
void handleKeyInput(GLFWwindow *window, int key, int scancode, int action, int mods);
void initMousePos(GLFWwindow *window);
bool outputFps(float dt, unsigned int &chunksPerSecond);
