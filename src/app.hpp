#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <map>
#include "camera.hpp"
#pragma once

enum KeyState {
	RELEASED,
	JUST_PRESSED,
	HELD,
};

class State {
	Camera cam;
	double mousex, mousey; //Mouse position
	glm::mat4 persp; //Global perspective matrix
	float currentFovy;
	float currentAspect;
	float currentZnear;
	float currentZfar;
	std::map<int, KeyState> keystates;
	GLFWwindow* window;
	State();
public:
	static State* get();
	double getMouseX();
	double getMouseY();
	void setMousePos(double x, double y);
	Camera& getCamera();
	void updatePerspectiveMat(float fovy, float znear, float zfar);
	glm::mat4 getPerspective();
	float getFovy();
	float getAspect();
	float getZnear();
	float getZfar();
	void setKey(int key, KeyState keystate);
	//Sets all the JUST_PRESSED keys to HELD
	void updateKeyStates();
	KeyState getKeyState(int key);
	GLFWwindow* getWindow();
	void createWindow(const char *name, int w, int h);
};

//Exit the program and output a message to stderr
void die(const char *msg);
//Window callback functions
void handleWindowResize(GLFWwindow *window, int w, int h);
void cursorPosCallback(GLFWwindow *window, double x, double y);
void handleKeyInput(GLFWwindow *window, int key, int scancode, int action, int mods);
//Initializes mouse position
void initMousePos(GLFWwindow *window);
//Returns true if the FPS is outputted
bool outputFps(float dt, unsigned int &chunksPerSecond);
//Initialize window and glad, if any of this fails, kill the program
//This should be called on window after glfwCreateWindow is called
void initWindow(GLFWwindow* window);
bool keyIsHeld(KeyState keystate);
