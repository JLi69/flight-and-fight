#include "game.hpp"
#include "app.hpp"
#include "audio.hpp"

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
		generateChunks(permutations, chunktables, RANGE);
		infworld::DecorationTable decorations = infworld::DecorationTable(14, CHUNK_SZ);
		decorations.genDecorations(permutations);
		gfx::generateDecorationOffsets(decorations);
		
		std::minstd_rand0 lcg;
		lcg.seed(randSeed);
		bool paused = false;
		bool stop = false;
		bool displaycrosshair = GlobalSettings::get()->values.canDisplayCrosshair;
		unsigned int score = 0;
		//Timers
		TimerManager timers;
		timers.addTimer("spawn_balloon", 0.0f, 20.0f);
		timers.addTimer("spawn_blimp", 20.0f, 45.0f);
		timers.addTimer("spawn_ufo", 45.0f, 60.0f);
		timers.addTimer("spawn_plane", 5.0f, 30.0f);
		//Gameobjects
		gobjs::Player player(glm::vec3(0.0f, HEIGHT * SCALE * 0.5f, 0.0f));
		std::vector<gobjs::Explosion> explosions;
		std::vector<gobjs::Enemy> balloons, blimps, ufos, planes;
		std::vector<gobjs::Bullet> bullets, enemybullets;

		unsigned int fps = 0;
		float dt = 0.0f;
		float totalTime = 0.0f;
		unsigned int chunksPerSecond = 0; //Number of chunks drawn per second	
		updateCamera(player);
		while(!glfwWindowShouldClose(state->getWindow()) && !stop) {
			float start = glfwGetTime();

			//Clear sound effect sources
			SNDSRC->clearSources();
			//Update listener
			audio::updateListener(
				player.transform.position, 
				player.transform.direction()
			);

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
			//Display blimps
			gfx::displayBlimps(blimps);
			//Display ufos
			gfx::displayUfos(ufos);
			//Display enemy planes
			gfx::displayPlanes(totalTime, planes);
			//Display bullets
			gfx::displayBullets(bullets);
			gfx::displayBullets(enemybullets);
			//Display water
			gfx::displayWater(totalTime);
			//Draw skybox
			gfx::displaySkybox();
			//Display explosions
			gfx::displayExplosions(explosions);
			//User Interface
			gui::displayFPSCounter(fps);
			gui::displayHUD(score, player.speed, player.hpPercent());
			glDisable(GL_CULL_FACE);
			glDepthMask(GL_FALSE);
			gfx::displayMiniMapBackground();
			gfx::displayEnemyMarkers(balloons, player.transform);
			gfx::displayEnemyMarkers(blimps, player.transform);
			gfx::displayEnemyMarkers(ufos, player.transform);
			gfx::displayEnemyMarkers(planes, player.transform);
			if(displaycrosshair)
				gfx::displayCrosshair(player.transform);
			glDepthMask(GL_TRUE);
			glEnable(GL_CULL_FACE);
			if(player.health > 0)
				gui::displayDamage(player.damageTimerProgress());
			
			if(player.crashed && !paused && player.deathtimer > 2.5f)
				gui::displayDeathScreen(score);
			
			bool prevpaused = paused;
			if(paused) {
				glfwSetInputMode(state->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				std::string action = gui::displayPauseMenu();
				if(action == "unpause") {
					paused = false;
					glfwSetInputMode(state->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
					state->clearInputState();
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

			//Just unpaused/paused
			if(prevpaused && !paused)
				SNDSRC->unpauseAll();
			else if(!prevpaused && paused)
				SNDSRC->pauseAll();

			if(!paused) {
				//Update timers
				timers.update(dt);

				//Toggle crosshair
				if(state->getKeyState(GLFW_KEY_T) == JUST_PRESSED)
					displaycrosshair = !displaycrosshair;
				//Shoot bullets
				KeyState leftbutton = state->getButtonState(GLFW_MOUSE_BUTTON_LEFT);
				KeyState spacebar = state->getKeyState(GLFW_KEY_SPACE);
				if(player.shoottimer <= 0.0f && 
				   (keyIsHeld(spacebar) || keyIsHeld(leftbutton)) &&
				   !player.crashed) {
					SNDSRC->playid("shoot", player.transform.position);
					player.resetShootTimer();
					bullets.push_back(gobjs::Bullet(player, glm::vec3(-8.5f, -0.75f, 8.5f)));
					bullets.push_back(gobjs::Bullet(player, glm::vec3(8.5f, -0.75f, 8.5f)));
				}
				//Update bullets
				checkBulletDist(bullets, player);
				updateBullets(bullets, dt);
				checkForBulletTerrainCollision(bullets, permutations);
				checkForHit(bullets, balloons, 24.0f);
				checkForHit(bullets, blimps, 32.0f);
				checkForHit(bullets, ufos, 14.0f);
				checkForHit(bullets, planes, 12.0f);
				//Update enemy bullets
				checkBulletDist(enemybullets, player);
				updateBullets(enemybullets, dt);
				checkForBulletTerrainCollision(enemybullets, permutations);
				checkForHit(enemybullets, player, 14.0f);
				//Spawn balloons
				if(timers.getTimer("spawn_balloon"))
					spawnBalloons(player, balloons, lcg, permutations);	
				if(timers.getTimer("spawn_blimp"))
					spawnBlimps(player, blimps, lcg);
				if(timers.getTimer("spawn_ufo"))
					spawnUfos(player, ufos, lcg, permutations);
				if(timers.getTimer("spawn_plane"))
					spawnPlanes(player, planes, lcg, permutations, totalTime);
				//Update balloons
				for(auto &balloon : balloons)
					balloon.updateBalloon(dt);
				//Update blimps
				for(auto &blimp : blimps)
					blimp.updateBlimp(dt);
				//Update ufos
				for(auto &ufo : ufos)
					ufo.updateUfo(dt, permutations);
				//Update enemy planes
				for(auto &plane : planes)
					plane.updatePlane(dt, player, enemybullets, permutations);
				//Update plane
				player.update(dt);
				bool justcrashed = player.crashed;
				player.checkIfCrashed(dt, permutations);
				//Check if any planes have collided with each other
				checkForCollision(planes, 16.0f);
				//Destroy any enemies that are too far away or have run out of health
				destroyEnemies(player, balloons, explosions, 1.0f, 24.0f, score);
				destroyEnemies(player, blimps, explosions, 2.5f, 36.0f, score);
				destroyEnemies(player, ufos, explosions, 1.0f, 14.0f, score);
				destroyEnemies(player, planes, explosions, 1.0f, 18.0f, score);
				checkForCollision(player, blimps, explosions, 2.5f, glm::vec3(26.0f, 26.0f, 72.0f));
				justcrashed = player.crashed ^ justcrashed;
				//Update explosions
				if(justcrashed) {
					explosions.push_back(gobjs::Explosion(player.transform.position));
					SNDSRC->playid("explosion", player.transform.position);	
				}
				updateExplosions(explosions, player.transform.position, dt);
				//Update camera
				updateCamera(player, dt);
				generateNewChunks(permutations, chunktables, decorations);

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
		SNDSRC->stopAll();
		for(int i = 0; i < MAX_LOD; i++)
			chunktables[i].clearBuffers();

		return score;
	}
}
