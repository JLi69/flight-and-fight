#pragma once

#include "infworld.hpp"

//Constants
constexpr float SPEED = 48.0f;
constexpr float BULLET_SPEED = 384.0f;
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

	struct Timer {
		float time = 0.0f;
		float maxtime = 0.0f;
	};

	struct TimerManager {
		std::unordered_map<std::string, Timer> timers;
		void addTimer(const std::string& name, float maxtime);
		void addTimer(const std::string& name, float time, float maxtime);
		void update(float dt);
		//Resets all timers that have time below 0.0
		void reset();
		bool getTimer(const std::string &name);
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
		float shoottimer = 0.0f;
		Player(glm::vec3 position);

		void update(float dt);
		void resetShootTimer();
		void checkIfCrashed(float dt, infworld::worldseed &permutations);
	};

	struct Explosion {
		game::Transform transform;
		float timePassed;
		bool visible;
		Explosion(glm::vec3 position);
		void update(float dt);
	};

	struct Enemy {
		game::Transform transform;
		std::unordered_map<std::string, float> values;
		int hitpoints;
		Enemy(glm::vec3 position, int hp);
		void updateBalloon(float dt);
		float getVal(const std::string &key);
		void setVal(const std::string &key, float v);
	};

	struct Bullet {
		bool destroyed = false;
		game::Transform transform;
		float time;
		Bullet(const Player &player, const glm::vec3 &offset);
		void update(float dt);
	};

	Enemy spawnBalloon(const glm::vec3 &position, infworld::worldseed &permutations);
}

namespace game {
	//Spawns balloons around the player
	void spawnBalloons(
		gameobjects::Player &player,
		std::vector<gameobjects::Enemy> &balloons,
		std::minstd_rand0 &lcg,
		infworld::worldseed &permutations
	);
	void destroyEnemies(
		gameobjects::Player &player,
		std::vector<gameobjects::Enemy> &enemies,
		std::vector<gameobjects::Explosion> &explosions,
		float crashdist
	);
	//Returns the position the camera should be following
	glm::vec3 getCameraFollowPos(const Transform &playertransform);
	//Have the camera follow the player
	void updateCamera(gameobjects::Player &player);
	//Update explosions
	void updateExplosions(
		std::vector<gameobjects::Explosion> &explosions, 
		const glm::vec3 &center,
		float dt
	);
	void updateBullets(std::vector<gameobjects::Bullet> &bullets, float dt);
	void checkForHit(
		std::vector<gameobjects::Bullet> &bullets,
		std::vector<gameobjects::Enemy> &enemies,
		float hitdist
	);
}

namespace gfx {
	void displaySkybox();
	void displayWater(float totalTime);
	void displayDecorations(infworld::DecorationTable &decorations, float totalTime);
	unsigned int displayTerrain(infworld::ChunkTable *chunktables, int maxlod, float lodscale);	
	void generateDecorationOffsets(infworld::DecorationTable &decorations);
	void displayPlayerPlane(float totalTime, const game::Transform &transform);
	void displayExplosions(const std::vector<gameobjects::Explosion> &explosions);
	void displayBalloons(const std::vector<gameobjects::Enemy> &balloons);
	void displayBullets(const std::vector<gameobjects::Bullet> &bullets);
	void displayMiniMapBackground();
	void displayEnemyMarkers(
		const std::vector<gameobjects::Enemy> &enemies,
		const game::Transform &playertransform
	);
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
