#include <glad/glad.h>
#include <stdio.h>
#include "infworld.hpp"
#include "gfx.hpp"
#include "app.hpp"
#include "plants.hpp"
#include "assets.hpp"
#include "game.hpp"
#include "hiscore.hpp"

const char highScoreTablePath[] = "hiscores";

int main(int argc, char *argv[])
{
	State* state = State::get();

	//Initialize glfw and glad, if any of this fails, kill the program
	if(!glfwInit()) 
		die("Failed to init glfw!");
	state->createWindow("Flight & Fight", 960, 720);
	initWindow(state->getWindow());
	state->initNuklear();

	game::loadAssets();	
	game::initUniforms();

	//High score table
	HighScoreTable highscores = loadHighScores(highScoreTablePath);

	//Set OpenGL state
	glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while(!glfwWindowShouldClose(state->getWindow())) {
		game::GameMode gamemode = game::mainMenu();
		glfwSetInputMode(state->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glBindVertexArray(0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		unsigned int score;
		switch(gamemode) {
			case game::HIGH_SCORE_SCREEN:
				game::highScoreTableScreen(highscores);
				break;
			case game::CASUAL:
				game::casualModeGameLoop();	
				break;
			case game::FIGHT:
				score = game::fightModeGameLoop();
				addHighScore(highscores, score);
				saveHighScores(highScoreTablePath, highscores);
				break;
			default:
				break;
		}
	}

	saveHighScores(highScoreTablePath, highscores);
	glfwTerminate();
}
