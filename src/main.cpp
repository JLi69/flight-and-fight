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

constexpr float SPEED = 32.0f;
constexpr float FLY_SPEED = 20.0f;
constexpr unsigned int MAX_LOD = 5;
constexpr float LOD_SCALE = 2.0f;

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

int main(int argc, char *argv[])
{
	Args argvals = parseArgs(argc, argv);

	State* state = State::get();
	Camera& cam = state->getCamera();

	printf("seed: %d\n", argvals.seed);
	infworld::worldseed permutations = infworld::makePermutations(argvals.seed, 9);

	//Initialize glfw and glad, if any of this fails, kill the program
	if(!glfwInit()) 
		die("Failed to init glfw!");
	GLFWwindow* window = glfwCreateWindow(960, 720, "infworld", NULL, NULL);
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

	infworld::ChunkTable chunktables[MAX_LOD];	
	generateChunks(permutations, chunktables, argvals.range);
	infworld::DecorationTable decorations = infworld::DecorationTable(32, CHUNK_SZ);
	decorations.genDecorations(permutations);
	//Quad
	gfx::Vao quad = gfx::createQuadVao();
	//Cube
	gfx::Vao cube = gfx::createCubeVao();
	//Trees
	gfx::Vao 
		pinetree = plants::createPineTreeModel(8),
		pinetreelowdetail = plants::createPineTreeModel(4);
	gfx::Vao 
		tree = plants::createTreeModel(6),
		treelowdetail = plants::createTreeModel(3);
	//Textures
	unsigned int terraintextures;
	glGenTextures(1, &terraintextures); 
	gfx::loadTexture("assets/textures/terraintextures.png", terraintextures); 
	unsigned int watermaps;
	glGenTextures(1, &watermaps);
	gfx::loadTexture("assets/textures/watermaps.png", watermaps);
	unsigned int pinetexture, treetexture;
	glGenTextures(1, &pinetexture);
	glGenTextures(1, &treetexture);	
	gfx::loadTexture("assets/textures/pinetreetexture.png", pinetexture);
	gfx::loadTexture("assets/textures/treetexture.png", treetexture);
	unsigned int skyboxcubemap;
	glGenTextures(1, &skyboxcubemap);
	const std::vector<std::string> faces = {
		"assets/textures/skybox/skybox_east.png",
		"assets/textures/skybox/skybox_west.png",
		"assets/textures/skybox/skybox_up.png",
		"assets/textures/skybox/skybox_down.png",
		"assets/textures/skybox/skybox_north.png",
		"assets/textures/skybox/skybox_south.png"
	};
	gfx::loadCubemap(faces, skyboxcubemap);

	ShaderProgram terrainShader("assets/shaders/terrainvert.glsl", "assets/shaders/terrainfrag.glsl");
	ShaderProgram waterShader("assets/shaders/instancedvert.glsl", "assets/shaders/waterfrag.glsl");
	ShaderProgram simpleWaterShader("assets/shaders/instancedvert.glsl", "assets/shaders/watersimplefrag.glsl");
	ShaderProgram skyboxShader("assets/shaders/skyboxvert.glsl", "assets/shaders/skyboxfrag.glsl");
	ShaderProgram treeShader("assets/shaders/tree-vert.glsl", "assets/shaders/textured-frag.glsl");
	float viewdist = CHUNK_SZ * SCALE * 2.0f * float(argvals.range) * std::pow(LOD_SCALE, MAX_LOD - 2);
	waterShader.use();
	waterShader.uniformFloat("viewdist", viewdist);
	simpleWaterShader.use();
	simpleWaterShader.uniformFloat("viewdist", viewdist);
	treeShader.use();
	treeShader.uniformFloat("viewdist", viewdist);
	terrainShader.use();
	terrainShader.uniformFloat("viewdist", viewdist);
	terrainShader.uniformFloat("maxheight", HEIGHT); 
	terrainShader.uniformInt("prec", PREC);
	
	decorations.generateOffsets(infworld::PINE_TREE, pinetree, 0, 5);
	decorations.generateOffsets(infworld::PINE_TREE, pinetreelowdetail, 5, 999);
	decorations.generateOffsets(infworld::TREE, tree, 0, 5);
	decorations.generateOffsets(infworld::TREE, treelowdetail, 5, 16);

	glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float dt = 0.0f;
	float time = 0.0f;
	unsigned int chunksPerSecond = 0; //Number of chunks drawn per second
	while(!glfwWindowShouldClose(window)) {
		float start = glfwGetTime();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Get perspective matrix
		int w, h;
		glfwGetWindowSize(window, &w, &h);
		const float aspect = float(w) / float(h);
		const float fovy = glm::radians(75.0f);
		glm::mat4 persp = glm::perspective(fovy, aspect, 2.0f, 20000.0f);
		//View matrix
		glm::mat4 view = cam.viewMatrix();
		geo::Frustum viewfrustum = cam.getViewFrustum(2.0f, 20000.0f, aspect, fovy);	

		//Draw terrain
		terrainShader.use();
		//Textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, terraintextures);
		terrainShader.uniformInt("terraintexture", 0);
		//uniforms
		terrainShader.uniformMat4x4("persp", persp);
		terrainShader.uniformMat4x4("view", view);
		terrainShader.uniformVec3("lightdir", glm::normalize(glm::vec3(-1.0f)));
		terrainShader.uniformVec3("camerapos", cam.position);
		terrainShader.uniformFloat("time", time);
		unsigned int drawCount = 0;	

		for(int i = 0; i < MAX_LOD; i++) {
			terrainShader.uniformFloat("chunksz", chunktables[i].scale());

			if(i < MAX_LOD - 1) {
				float chunkscale = 
					chunktables[i + 1].scale() * 
					2.0f * 
					float(PREC) / float(PREC + 1);
				float range = float(chunktables[i + 1].range()) / LOD_SCALE - 0.5f;
				//Slight amount of overlap to mitigate cracks in terrain
				//We increase this amount due to the terrain becoming less precise
				//and more likely to have cracks
				float d = 8.0f * float(i) + 4.0f;
				float maxrange = chunkscale * range * SCALE + d;
				infworld::ChunkPos center = chunktables[i + 1].getCenter();
				
				glm::vec2 centerpos = glm::vec2(float(center.z), float(center.x));
				centerpos *= float(PREC) / float(PREC + 1);
				centerpos *= chunktables[i + 1].scale() * SCALE * 2.0f;
				
				terrainShader.uniformFloat("maxrange", maxrange);
				terrainShader.uniformVec2("center", centerpos);
			}
			else {
				terrainShader.uniformVec2("center", glm::vec2(0.0f));
				terrainShader.uniformFloat("maxrange", -1.0f);
			}

			if(i == 0)
				drawCount += chunktables[i].draw(terrainShader, viewfrustum);
			else {
				int minrange = chunktables[i - 1].range() / int(LOD_SCALE);
				drawCount += chunktables[i].draw(terrainShader, minrange, viewfrustum);
			}
		}
		chunksPerSecond += drawCount;

		glDisable(GL_CULL_FACE);
		//Display trees	
		treeShader.use();
		treeShader.uniformMat4x4("persp", persp);
		treeShader.uniformMat4x4("view", view);
		treeShader.uniformVec3("lightdir", glm::normalize(glm::vec3(-1.0f)));
		treeShader.uniformVec3("camerapos", cam.position);
		treeShader.uniformFloat("time", time);
		treeShader.uniformFloat("windstrength", SCALE * 3.0f);
		treeShader.uniformMat4x4(
			"transform",
			glm::scale(glm::mat4(1.0f), glm::vec3(SCALE * 2.5f))
		);
		treeShader.uniformVec3("camerapos", cam.position);
		//Draw pine trees
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pinetexture);
		pinetree.bind();
		decorations.drawDecorations(pinetree);
		pinetreelowdetail.bind();
		decorations.drawDecorations(pinetreelowdetail);
		//Draw trees
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, treetexture);
		tree.bind();
		decorations.drawDecorations(tree);
		treelowdetail.bind();
		decorations.drawDecorations(treelowdetail);

		quad.bind();
		const int waterrange = 4;
		const int count = (waterrange * 2 + 1) * (waterrange * 2 + 1);
		const float quadscale = CHUNK_SZ * 32.0f * SCALE;
		//Draw water
		waterShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, watermaps);
		waterShader.uniformInt("range", waterrange);
		waterShader.uniformFloat("scale", quadscale);
		waterShader.uniformInt("waternormals", 0);
		waterShader.uniformInt("waterdudv", 1);
		waterShader.uniformMat4x4("persp", persp);
		waterShader.uniformMat4x4("view", view);
		waterShader.uniformVec3("lightdir", glm::normalize(glm::vec3(-1.0f)));
		waterShader.uniformVec3("camerapos", cam.position);
		waterShader.uniformFloat("time", time);
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, glm::vec3(cam.position.x, 0.0f, cam.position.z));
		transform = glm::scale(transform, glm::vec3(quadscale));
		waterShader.uniformMat4x4("transform", transform);
		glDrawElementsInstanced(GL_TRIANGLES, quad.vertcount, GL_UNSIGNED_INT, 0, count);
		glEnable(GL_CULL_FACE);

		//Draw skybox
		glCullFace(GL_FRONT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxcubemap);
		skyboxShader.use();
		skyboxShader.uniformInt("skybox", 0);
		skyboxShader.uniformMat4x4("persp", persp);
		glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
		skyboxShader.uniformMat4x4("view", skyboxView);
		cube.bind();
		glDrawElements(GL_TRIANGLES, cube.vertcount, GL_UNSIGNED_INT, 0);
		glCullFace(GL_BACK);

		//Update camera
		cam.position += cam.velocity() * dt * SPEED;
		cam.fly(dt, FLY_SPEED);
		for(int i = 0; i < MAX_LOD; i++)
			chunktables[i].generateNewChunks(cam.position.x, cam.position.z, permutations);
		bool generated = decorations.genNewDecorations(cam.position.x, cam.position.z, permutations);
		if(generated) {
			decorations.generateOffsets(infworld::PINE_TREE, pinetree, 0, 5);
			decorations.generateOffsets(infworld::PINE_TREE, pinetreelowdetail, 5, 999);
			decorations.generateOffsets(infworld::TREE, tree, 0, 5);
			decorations.generateOffsets(infworld::TREE, treelowdetail, 5, 16);
		}

		glfwSwapBuffers(window);
		gfx::outputErrors();
		glfwPollEvents();
		time += dt;
		outputFps(dt, chunksPerSecond);
		dt = glfwGetTime() - start;
	}

	//Clean up	
	for(int i = 0; i < MAX_LOD; i++)
		chunktables[i].clearBuffers();
	gfx::destroyVao(quad);
	glfwTerminate();
}
