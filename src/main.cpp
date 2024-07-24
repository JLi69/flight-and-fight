#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

#include "shader.hpp"
#include "camera.hpp"
#include "infworld.hpp"
#include "gfx.hpp"
#include "app.hpp"
#include "arg.hpp"
#include "plants.hpp"
#include "assets.hpp"
#include "display.hpp"

constexpr float SPEED = 32.0f;
constexpr float FLY_SPEED = 20.0f;
constexpr unsigned int MAX_LOD = 5;
constexpr float LOD_SCALE = 2.0f;

constexpr float FOVY = glm::radians(75.0f);
constexpr float ZNEAR = 2.0f;
constexpr float ZFAR = 20000.0f;

const glm::vec3 LIGHT = glm::normalize(glm::vec3(-1.0f));

void generateChunks(
	const infworld::worldseed &permutations,
	infworld::ChunkTable *chunktables,
	unsigned int range
) {
	float sz = CHUNK_SZ;
	for(int i = 0; i < MAX_LOD; i++) {
		chunktables[i] = 
			infworld::buildWorld(range, permutations, HEIGHT, sz);
		sz *= LOD_SCALE;
	}
}

void generateDecorationOffsets(infworld::DecorationTable &decorations)
{
	decorations.generateOffsets(infworld::PINE_TREE, VAOS->getVao("pinetree"), 0, 5);
	decorations.generateOffsets(infworld::PINE_TREE, VAOS->getVao("pinetreelowdetail"), 5, 999);
	decorations.generateOffsets(infworld::TREE, VAOS->getVao("tree"), 0, 5);
	decorations.generateOffsets(infworld::TREE, VAOS->getVao("treelowdetail"), 5, 16);
}

void generateNewChunks(
	const infworld::worldseed &permutations,
	infworld::ChunkTable *chunktables,
	infworld::DecorationTable &decorations
) {
	Camera& cam = State::get()->getCamera();
	for(int i = 0; i < MAX_LOD; i++)
		chunktables[i].generateNewChunks(cam.position.x, cam.position.z, permutations);
	bool generated = decorations.genNewDecorations(cam.position.x, cam.position.z, permutations);
	if(generated)
		generateDecorationOffsets(decorations);
}

int main(int argc, char *argv[])
{
	Args argvals = parseArgs(argc, argv);

	State* state = State::get();
	Camera& cam = state->getCamera();

	infworld::worldseed permutations = infworld::makePermutations(argvals.seed, 9);

	//Initialize glfw and glad, if any of this fails, kill the program
	if(!glfwInit()) 
		die("Failed to init glfw!");
	GLFWwindow* window = glfwCreateWindow(960, 720, "flight sim", NULL, NULL);
	if(!window)
		die("Failed to create window!");
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glfwSetWindowSizeCallback(window, handleWindowResize);
	glfwSetKeyCallback(window, handleKeyInput);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		die("Failed to init glad!");
	initMousePos(window);
	
	//Vaos
	VAOS->genSimple();
	VAOS->add("pinetree", plants::createPineTreeModel(8));
	VAOS->add("pinetreelowdetail", plants::createPineTreeModel(4));
	VAOS->add("tree", plants::createTreeModel(6));
	VAOS->add("treelowdetail", plants::createTreeModel(3));
	VAOS->importFromFile("assets/models.impfile");
	//Textures
	TEXTURES->importFromFile("assets/textures.impfile");
	//Shaders
	SHADERS->importFromFile("assets/shaders.impfile");
	const float viewdist = 
		CHUNK_SZ * SCALE * 2.0f * float(argvals.range) * std::pow(LOD_SCALE, MAX_LOD - 2);
	SHADERS->use("water");
	SHADERS->getShader("water").uniformFloat("viewdist", viewdist);
	SHADERS->use("tree");
	SHADERS->getShader("tree").uniformFloat("viewdist", viewdist);
	SHADERS->use("terrain");
	SHADERS->getShader("terrain").uniformFloat("viewdist", viewdist);
	SHADERS->getShader("terrain").uniformFloat("maxheight", HEIGHT);
	SHADERS->getShader("terrain").uniformInt("prec", PREC);
	
	//Initially generate world
	infworld::ChunkTable chunktables[MAX_LOD];
	generateChunks(permutations, chunktables, argvals.range);
	infworld::DecorationTable decorations = infworld::DecorationTable(32, CHUNK_SZ);
	decorations.genDecorations(permutations);
	generateDecorationOffsets(decorations);

	glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float dt = 0.0f;
	float totalTime = 0.0f;
	unsigned int chunksPerSecond = 0; //Number of chunks drawn per second
	while(!glfwWindowShouldClose(window)) {
		float start = glfwGetTime();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Update perspective matrix
		state->updatePerspectiveMat(window, FOVY, ZNEAR, ZFAR);

		//Draw terrain
		unsigned int drawCount = gfx::displayTerrain(chunktables, MAX_LOD, LOD_SCALE);
		chunksPerSecond += drawCount;

		//Display trees	
		gfx::displayDecorations(decorations, totalTime);
		
		//Display water
		gfx::displayWater(totalTime);

		//Display plane	
		ShaderProgram& shader = SHADERS->getShader("textured");	

		shader.use();
		shader.uniformMat4x4("persp", state->getPerspective());
		shader.uniformMat4x4("view", cam.viewMatrix());
		shader.uniformVec3("lightdir", LIGHT);
		shader.uniformVec3("camerapos", cam.position);
		
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, glm::vec3(0.0f, HEIGHT * SCALE * 0.5f, 0.0f));
		glm::mat4 normal = glm::mat3(glm::transpose(glm::inverse(transform)));
		TEXTURES->bindTexture("plane", GL_TEXTURE0);	
		shader.uniformFloat("specularfactor", 0.5f);	
		shader.uniformMat4x4("transform", transform);
		shader.uniformMat3x3("normalmat", normal);
		VAOS->bind("plane");
		VAOS->draw();

		TEXTURES->bindTexture("propeller", GL_TEXTURE0);
		glm::mat4 propellerTransform = glm::mat4(1.0f);
		propellerTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 13.888f));
		float rotation = totalTime * 16.0f;
		propellerTransform = glm::rotate(propellerTransform, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
		propellerTransform = transform * propellerTransform;
		normal = glm::mat3(glm::transpose(glm::inverse(propellerTransform)));
		shader.uniformFloat("specularfactor", 0.0f);
		shader.uniformMat4x4("transform", propellerTransform);
		shader.uniformMat3x3("normalmat", normal);
		VAOS->bind("propeller");
		VAOS->draw();

		//Draw skybox
		gfx::displaySkybox();	

		//Update camera
		cam.position += cam.velocity() * dt * SPEED;
		cam.fly(dt, FLY_SPEED);
		generateNewChunks(permutations, chunktables, decorations);

		glfwSwapBuffers(window);
		gfx::outputErrors();
		glfwPollEvents();
		totalTime += dt;
		outputFps(dt, chunksPerSecond);
		dt = glfwGetTime() - start;
	}

	//Clean up	
	for(int i = 0; i < MAX_LOD; i++)
		chunktables[i].clearBuffers();
	glfwTerminate();
}
