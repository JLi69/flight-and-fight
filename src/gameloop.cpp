#include "game.hpp"
#include "app.hpp"
#include <GLFW/glfw3.h>

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

		//Gameobjects
		gobjs::Player player(glm::vec3(0.0f, HEIGHT * SCALE * 0.5f, 0.0f));
		std::vector<gobjs::Explosion> explosions;

		glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		float dt = 0.0f;
		float totalTime = 0.0f;
		unsigned int chunksPerSecond = 0; //Number of chunks drawn per second
		while(!glfwWindowShouldClose(state->getWindow())) {
			float start = glfwGetTime();

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
			game::updateCamera(player);
			game::generateNewChunks(permutations, chunktables, decorations);	

			state->updateKeyStates();
			glfwSwapBuffers(state->getWindow());
			gfx::outputErrors();
			glfwPollEvents();
			totalTime += dt;
			outputFps(dt, chunksPerSecond);
			dt = glfwGetTime() - start;
		}

		//Clean up	
		for(int i = 0; i < MAX_LOD; i++)
			chunktables[i].clearBuffers();
	}
}
