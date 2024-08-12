#include "game.hpp"
#include "app.hpp"

namespace gobjs = gameobjects;

namespace game {
	game::GameMode mainMenu()
	{
		State* state = State::get();
		bool showCredits = false;
		std::vector<std::string> credits = gui::readTextFile("assets/credits.txt");

		glfwSetInputMode(state->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		state->getCamera().pitch = 0.0f;
		state->getCamera().yaw = 0.0f;
		state->getCamera().position = glm::vec3(0.0f);

		gobjs::Player player(glm::vec3(14.0f, -8.0f, -40.0f));
		
		float dt = 0.0f;
		float totalTime = 0.0f;
		while(!glfwWindowShouldClose(state->getWindow())) {
			float start = glfwGetTime();

			nk_glfw3_new_frame(state->getNkGlfw());

			//Update perspective matrix
			state->updatePerspectiveMat(FOVY, ZNEAR, ZFAR);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			gfx::displayPlayerPlane(totalTime, player.transform);
			//Display skybox
			gfx::displaySkybox();

			game::GameMode selected = gui::displayMainMenu();
			if(showCredits) {
				bool close = gui::displayCredits(credits);
				if(close)
					showCredits = false;
			}

			player.transform.rotation.y += 3.14f * dt;

			state->updateKeyStates();
			nk_glfw3_render(state->getNkGlfw(), NK_ANTI_ALIASING_ON, 512 * 1024, 128 * 1024);
			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glfwSwapBuffers(state->getWindow());
			glfwPollEvents();
			gfx::outputErrors();
			totalTime += dt;

			switch(selected) {
			case CASUAL: //Fall through
			case FIGHT:
				return selected;
			case CREDITS:
				showCredits = true;
			default:
				break;
			}

			dt = glfwGetTime() - start;
		}

		return NONE_SELECTED;
	}	
}
