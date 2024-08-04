#pragma once

#include <glad/glad.h>
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_KEYSTATE_BASED_INPUT
#include <nuklear/nuklear.h>
#include <nuklear/nuklear_glfw_gl3.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <map>
#include "camera.hpp"

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
	nk_glfw glfw = {0};
	nk_context* ctx;
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
	void initNuklear();
	nk_glfw* getNkGlfw();
	nk_context* getNkContext();
};

//Exit the program and output a message to stderr
void die(const char *msg);
//Window callback functions
void handleWindowResize(GLFWwindow *window, int w, int h);
void cursorPosCallback(GLFWwindow *window, double x, double y);
void handleKeyInput(GLFWwindow *window, int key, int scancode, int action, int mods);
//Initializes mouse position
void initMousePos(GLFWwindow *window);
//Returns the FPS
unsigned int outputFps(float dt, unsigned int &chunksPerSecond);
//Initialize window and glad, if any of this fails, kill the program
//This should be called on window after glfwCreateWindow is called
void initWindow(GLFWwindow* window);
bool keyIsHeld(KeyState keystate);
