#include "game.hpp"
#include "app.hpp"

namespace gobjs = gameobjects;

namespace game {
	void casualModeGameLoop()
	{
		State* state = State::get();

		//Initially generate world
		std::random_device rd;
		int randSeed = rd();
		infworld::worldseed permutations = infworld::makePermutations(randSeed, 9);
		infworld::ChunkTable chunktables[MAX_LOD];
		game::generateChunks(permutations, chunktables, RANGE);
		infworld::DecorationTable decorations = infworld::DecorationTable(14, CHUNK_SZ);
		decorations.genDecorations(permutations);
		gfx::generateDecorationOffsets(decorations);

		bool paused = false;
		bool stop = false;
		//Gameobjects
		gobjs::Player player(glm::vec3(0.0f, HEIGHT * SCALE * 0.5f, 0.0f));
		std::vector<gobjs::Explosion> explosions;

		unsigned int fps = 0;
		float dt = 0.0f;
		float totalTime = 0.0f;
		unsigned int chunksPerSecond = 0; //Number of chunks drawn per second	
		game::updateCamera(player);
		while(!glfwWindowShouldClose(state->getWindow()) && !stop) {
			float start = glfwGetTime();

			nk_glfw3_new_frame(state->getNkGlfw());

			//Update perspective matrix
			state->updatePerspectiveMat(FOVY, ZNEAR, ZFAR);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//Draw terrain
			unsigned int drawCount = gfx::displayTerrain(chunktables, MAX_LOD, LOD_SCALE);
			chunksPerSecond += drawCount;
			//Display trees	
			gfx::displayDecorations(decorations, totalTime);	
			//Display plane
			if(!player.crashed)
				gfx::displayPlayerPlane(totalTime, player.transform);		
			//Display water
			gfx::displayWater(totalTime);	
			//Draw skybox
			gfx::displaySkybox();
			//Display explosions
			gfx::displayExplosions(explosions);
			//User Interface
			gui::displayFPSCounter(fps);
			if(player.crashed && !paused && player.deathtimer > 2.5f)
				gui::displayDeathScreen(0);
			if(paused) {
				glfwSetInputMode(state->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				std::string action = gui::displayPauseMenu();
				if(action == "unpause") {
					paused = false;
					glfwSetInputMode(state->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				}
				else if(action == "quit") {
					stop = true;
					glfwSetInputMode(state->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
			}
			else if(!paused)
				glfwSetInputMode(state->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			
			//Pause/unpause the game
			if(state->getKeyState(GLFW_KEY_ESCAPE) == JUST_PRESSED)
				paused = !paused;
			if(!paused) {
				//Update plane
				player.update(dt);
				bool justcrashed = player.crashed;
				player.checkIfCrashed(dt, permutations);
				justcrashed = player.crashed ^ justcrashed;
				//Update explosions
				if(justcrashed)
					explosions.push_back(gobjs::Explosion(player.transform.position));
				for(auto &explosion : explosions)
					explosion.update(dt);
				//Update camera
				game::updateCamera(player, dt);
				game::generateNewChunks(permutations, chunktables, decorations);
			
				totalTime += dt;
			}


			state->updateKeyStates();
			nk_glfw3_render(state->getNkGlfw(), NK_ANTI_ALIASING_ON, 512 * 1024, 128 * 1024);
			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glfwSwapBuffers(state->getWindow());
			glfwPollEvents();
			gfx::outputErrors();
			fps = outputFps(dt, chunksPerSecond);	
			dt = glfwGetTime() - start;
		}

		//Clean up	
		for(int i = 0; i < MAX_LOD; i++)
			chunktables[i].clearBuffers();
	}
}
