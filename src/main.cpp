#include <glad/glad.h>
#include <stdio.h>

#include "camera.hpp"
#include "infworld.hpp"
#include "gfx.hpp"
#include "app.hpp"
#include "plants.hpp"
#include "assets.hpp"
#include "game.hpp"

namespace gobjs = gameobjects;

int main(int argc, char *argv[])
{
	State* state = State::get();

	//Initialize glfw and glad, if any of this fails, kill the program
	if(!glfwInit()) 
		die("Failed to init glfw!");
	state->createWindow("flight sim", 960, 720);
	initWindow(state->getWindow());
	state->initNuklear();

	game::loadAssets();
	//Load fonts
	nk_font_atlas *fonts;
	nk_glfw3_font_stash_begin(state->getNkGlfw(), &fonts);
	nk_glfw3_font_stash_end(state->getNkGlfw());
	//Initialize uniforms - TODO: these can probably be made into a single
	//uniform buffer object and then they can be accessed across all shaders
	const float viewdist = 
		CHUNK_SZ * SCALE * 2.0f * float(RANGE) * std::pow(LOD_SCALE, MAX_LOD - 2);
	SHADERS->use("water");
	SHADERS->getShader("water").uniformFloat("viewdist", viewdist);
	SHADERS->use("tree");
	SHADERS->getShader("tree").uniformFloat("viewdist", viewdist);
	SHADERS->use("terrain");
	SHADERS->getShader("terrain").uniformFloat("viewdist", viewdist);
	SHADERS->getShader("terrain").uniformFloat("maxheight", HEIGHT);
	SHADERS->getShader("terrain").uniformInt("prec", PREC);

	//Set OpenGL state
	glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	game::casualModeGameLoop();	

	glfwTerminate();
}
