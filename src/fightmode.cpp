#include "game.hpp"
#include "app.hpp"

namespace gobjs = gameobjects;

namespace game {
	unsigned int fightModeGameLoop()
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
		
		std::minstd_rand0 lcg;
		lcg.seed(randSeed);
		bool paused = false;
		bool stop = false;
		unsigned int score = 0;
		//Timers
		TimerManager timers;
		timers.addTimer("spawn_balloon", 0.0f, 20.0f);
		//Gameobjects
		gobjs::Player player(glm::vec3(0.0f, HEIGHT * SCALE * 0.5f, 0.0f));
		std::vector<gobjs::Explosion> explosions;
		std::vector<gobjs::Enemy> balloons;
		std::vector<gobjs::Bullet> bullets;

		unsigned int fps = 0;
		float dt = 0.0f;
		float totalTime = 0.0f;
		unsigned int chunksPerSecond = 0; //Number of chunks drawn per second	
		game::updateCamera(player);
		state->clearMouseState();
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
			//Display balloons
			gfx::displayBalloons(balloons);
			//Display bullets
			gfx::displayBullets(bullets);
			//Display water
			gfx::displayWater(totalTime);
			//Draw skybox
			gfx::displaySkybox();
			//Display explosions
			gfx::displayExplosions(explosions);
			//User Interface
			gui::displayFPSCounter(fps);
			gui::displayHUD(score, player.speed);
			glDisable(GL_CULL_FACE);
			glDepthMask(GL_FALSE);
			gfx::displayMiniMapBackground();
			gfx::displayEnemyMarkers(balloons, player.transform);
			glDepthMask(GL_TRUE);
			glEnable(GL_CULL_FACE);
			
			if(player.crashed && !paused && player.deathtimer > 2.5f)
				gui::displayDeathScreen(score);
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
				//Update timers
				timers.update(dt);

				//Shoot bullets
				KeyState leftbutton = state->getButtonState(GLFW_MOUSE_BUTTON_LEFT);
				KeyState spacebar = state->getKeyState(GLFW_KEY_SPACE);
				if(player.shoottimer <= 0.0f && (keyIsHeld(spacebar) || keyIsHeld(leftbutton))) {
					player.resetShootTimer();
					bullets.push_back(gobjs::Bullet(player, glm::vec3(-8.5f, -0.75f, 8.5f)));
					bullets.push_back(gobjs::Bullet(player, glm::vec3(8.5f, -0.75f, 8.5f)));
				}
				//Update bullets
				updateBullets(bullets, dt);
				checkForBulletTerrainCollision(bullets, permutations);
				checkForHit(bullets, balloons, 24.0f);
				//Spawn balloons
				if(timers.getTimer("spawn_balloon"))
					spawnBalloons(player, balloons, lcg, permutations);	
				//Update balloons
				for(auto &balloon : balloons)
					balloon.updateBalloon(dt);
				//Update plane
				player.update(dt);
				bool justcrashed = player.crashed;
				player.checkIfCrashed(dt, permutations);
				//Destroy any balloons that are too far away or are destroyed
				destroyEnemies(player, balloons, explosions, 24.0f, score);
				justcrashed = player.crashed ^ justcrashed;
				//Update explosions
				if(justcrashed)
					explosions.push_back(gobjs::Explosion(player.transform.position));
				game::updateExplosions(explosions, player.transform.position, dt);
				//Update camera
				game::updateCamera(player);
				game::generateNewChunks(permutations, chunktables, decorations);

				totalTime += dt;
				timers.reset();
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

		return score;
	}
}
