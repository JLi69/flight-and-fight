#pragma once

#include "infworld.hpp"

//Constants
constexpr float SPEED = 48.0f;
constexpr unsigned int MAX_LOD = 5;
constexpr float LOD_SCALE = 2.0f;

constexpr float FOVY = glm::radians(75.0f);
constexpr float ZNEAR = 2.0f;
constexpr float ZFAR = 20000.0f;

constexpr int RANGE = 4;

const glm::vec3 LIGHT = glm::normalize(glm::vec3(-1.0f));

namespace game {
	enum GameMode {
		CASUAL,
		FIGHT,
		CREDITS,
		NONE_SELECTED,
	};

	struct Transform {
		glm::vec3 position;
		glm::vec3 scale;
		glm::vec3 rotation; //in radians
		Transform();
		glm::mat4 getTransformMat() const;
		glm::vec3 direction() const; //Forward vector
		glm::vec3 right() const;
		glm::vec3 rotate(const glm::vec3 &v) const;
	};	

	void loadAssets();
	//Initializes the shader uniforms
	void initUniforms();
	void generateChunks(
		const infworld::worldseed &permutations,
		infworld::ChunkTable *chunktables,
		unsigned int range
	);
	void generateNewChunks(
		const infworld::worldseed &permutations,
		infworld::ChunkTable *chunktables,
		infworld::DecorationTable &decorations
	);

	//This is the game loop for "Casual Mode"
	//In casual mode, you simply fly your plane around to explore the world
	//and avoid crashing into the terrain
	void casualModeGameLoop();
	//This is the game loop for "Fight Mode"
	//In fight mode, there are other things in the sky you need to shoot down
	//and some of those things will shoot back at you so you must try to
	//destroy as many things as possible before being shot down yourself
	//returns the final score the player acheives
	unsigned int fightModeGameLoop();
	//Main menu
	//Returns the game mode selected
	game::GameMode mainMenu();
}

namespace gameobjects {
	struct Player {
		game::Transform transform;
		bool crashed;
		enum {
			RY_LEFT,
			RY_RIGHT,
			RY_NONE,
		} yRotationDirection;
		enum {
			RX_UP,
			RX_DOWN,
			RX_NONE,
		} xRotationDirection;
		float deathtimer = 0.0f; //Keeps track of how long the player has been dead
		Player(glm::vec3 position);

		void update(float dt);
		void checkIfCrashed(float dt, infworld::worldseed &permutations);
	};

	struct Explosion {
		game::Transform transform;
		float timePassed;
		bool visible;
		Explosion(glm::vec3 position);
		void update(float dt);
	};
}

namespace game {
	//Returns the position the camera should be following
	glm::vec3 getCameraFollowPos(const Transform &playertransform);
	//Have the camera follow the player
	void updateCamera(gameobjects::Player &player);
}

namespace gfx {
	void displaySkybox();
	void displayWater(float totalTime);
	void displayDecorations(infworld::DecorationTable &decorations, float totalTime);
	unsigned int displayTerrain(infworld::ChunkTable *chunktables, int maxlod, float lodscale);	
	void generateDecorationOffsets(infworld::DecorationTable &decorations);
	void displayPlayerPlane(float totalTime, const game::Transform &transform);
	void displayExplosions(const std::vector<gameobjects::Explosion> &explosions);
}

namespace gui {
	std::vector<std::string> readTextFile(const char *path);
	void displayFPSCounter(unsigned int fps);
	//Returns action taken by the user on the pause menu
	std::string displayPauseMenu();
	//Returns the action taken by the user on the death screen
	void displayDeathScreen();
	//returns the selected game mode
	game::GameMode displayMainMenu();
	//Display credits
	//returns true if user closed out of credits, false otherwise
	bool displayCredits(const std::vector<std::string> &credits);
}
