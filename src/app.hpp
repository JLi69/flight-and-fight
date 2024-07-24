#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "camera.hpp"
#pragma once

class State {
	Camera cam;
	double mousex, mousey; //Mouse position
	glm::mat4 persp; //Global perspective matrix
	float currentFovy;
	float currentAspect;
	float currentZnear;
	float currentZfar;
	State();
public:
	static State* get();
	double getMouseX();
	double getMouseY();
	void setMousePos(double x, double y);
	Camera& getCamera();
	void updatePerspectiveMat(GLFWwindow *window, float fovy, float znear, float zfar);
	glm::mat4 getPerspective();
	float getFovy();
	float getAspect();
	float getZnear();
	float getZfar();
};

void die(const char *msg);
void handleWindowResize(GLFWwindow *window, int w, int h);
void cursorPosCallback(GLFWwindow *window, double x, double y);
void handleKeyInput(GLFWwindow *window, int key, int scancode, int action, int mods);
void initMousePos(GLFWwindow *window);
bool outputFps(float dt, unsigned int &chunksPerSecond);
