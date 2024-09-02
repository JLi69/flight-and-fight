#pragma once

#include "infworld.hpp"
#include "hiscore.hpp"
#include "settings.hpp"

//Constants
constexpr float SPEED = 48.0f;
constexpr float ACCELERATION = 12.0f;
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
		HIGH_SCORE_SCREEN,
		SETTINGS,
		NONE_SELECTED,
	};

	enum SettingsScreenAction {
		SAVE_SETTINGS,
		CLEAR_SETTINGS,
		SETTINGS_NONE_SELECTED,
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
		glm::vec3 invRotate(const glm::vec3 &v) const;
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
	//High Score Table screen
	void highScoreTableScreen(const HighScoreTable &highscores);
	//Settings screen	
	void settingsScreen();
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
		float speed = 0.0f;
		//This is used to keep track of the time left for displaying a red screen
		//to indicate to the player that they took damage
		float damagetimer = 0.0f;
		//This keeps track of how long the player has damage immunity
		float damagecooldown = 0.0f;
		unsigned int health;
		Player(glm::vec3 position);

		void damage(unsigned int amount);
		//Returns the percentage of health left, rounded down
		unsigned int hpPercent();
		//Returns the damagetimer amount left
		float damageTimerProgress();
		void rotateWithMouse(float dt);
		void update(float dt);
		void resetShootTimer();
		void checkIfCrashed(float dt, const infworld::worldseed &permutations);
	};

	struct Explosion {
		game::Transform transform;
		float timePassed;
		float explosionScale;
		bool visible;
		Explosion(glm::vec3 position);
		Explosion(glm::vec3 position, float scale);
		void update(float dt);
	};

	struct Bullet {
		bool destroyed = false;
		game::Transform transform;
		float time;
		float speed;
		Bullet(const Player &player, const glm::vec3 &offset);
		Bullet(const game::Transform &t, float addspeed, const glm::vec3 &offset);
		Bullet();
		void update(float dt);
	};

	struct Enemy {
		//How many points the player gets if they kill the enemy
		unsigned int scorevalue;
		game::Transform transform;
		std::unordered_map<std::string, float> values;
		int hitpoints;
		Enemy(glm::vec3 position, int hp, unsigned int scoreval);
		void updateBalloon(float dt);
		void updateBlimp(float dt);
		void updateUfo(float dt, const infworld::worldseed &permutations);
		void updatePlane(
			float dt,
			const Player &player,
			std::vector<Bullet> &bullets,
			const infworld::worldseed &permutations
		);
		void checkIfPlaneCrashed(const infworld::worldseed &permutations);
		float getVal(const std::string &key) const;
		void setVal(const std::string &key, float v);
	};

	Enemy spawnBalloon(const glm::vec3 &position, const infworld::worldseed &permutations);
	Enemy spawnBlimp(const glm::vec3 &position, float rotation);
	Enemy spawnUfo(
		const glm::vec3 &position,
		float rotation,
		const infworld::worldseed &permutations
	);
	Enemy spawnPlane(
		const glm::vec3 &position,
		float rotation,
		const infworld::worldseed &permutations
	);
}

namespace game {
	//Spawns balloons around the player
	void spawnBalloons(
		gameobjects::Player &player,
		std::vector<gameobjects::Enemy> &balloons,
		std::minstd_rand0 &lcg,
		const infworld::worldseed &permutations
	);
	//Spawns blimps around the player
	void spawnBlimps(
		gameobjects::Player &player,
		std::vector<gameobjects::Enemy> &blimps,
		std::minstd_rand0 &lcg
	);
	//Spawn ufos around the player
	void spawnUfos(
		gameobjects::Player &player,
		std::vector<gameobjects::Enemy> &ufos,
		std::minstd_rand0 &lcg,
		const infworld::worldseed &permutations
	);
	//Spawn planes around the player
	void spawnPlanes(
		gameobjects::Player &player,
		std::vector<gameobjects::Enemy> &planes,
		std::minstd_rand0 &lcg,
		const infworld::worldseed &permutations,
		float totalTime
	);
	void destroyEnemies(
		gameobjects::Player &player,
		std::vector<gameobjects::Enemy> &enemies,
		std::vector<gameobjects::Explosion> &explosions,
		float explosionscale,
		float crashdist,
		unsigned int &score
	);
	void checkForCollision(
		gameobjects::Player &player,
		std::vector<gameobjects::Enemy> &enemies,
		std::vector<gameobjects::Explosion> &explosions,
		float explosionscale,
		const glm::vec3 &extents
	);
	//Check for collision among enemies
	void checkForCollision(std::vector<gameobjects::Enemy> &enemies, float hitdist);
	//Returns the position the camera should be following
	glm::vec3 getCameraFollowPos(const Transform &playertransform);
	//Have the camera follow the player	
	void updateCamera(gameobjects::Player &player);
	void updateCamera(gameobjects::Player &player, float dt);
	//Update explosions
	void updateExplosions(
		std::vector<gameobjects::Explosion> &explosions, 
		const glm::vec3 &center,
		float dt
	);
	//Check if the bullet is too far away from the player, if it is, then
	//mark it to be deleted
	//This should preferably be called before `updateBullets`
	void checkBulletDist(
		std::vector<gameobjects::Bullet> &bullets,
		const gameobjects::Player &player
	);
	void updateBullets(std::vector<gameobjects::Bullet> &bullets, float dt);
	void checkForHit(
		std::vector<gameobjects::Bullet> &bullets,
		std::vector<gameobjects::Enemy> &enemies,
		float hitdist
	);
	void checkForHit(
		std::vector<gameobjects::Bullet> &bullets,
		gameobjects::Player &player,
		float hitdist
	);
	void checkForBulletTerrainCollision(
		std::vector<gameobjects::Bullet> &bullets,
		infworld::worldseed &permutations
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
	void displayBlimps(const std::vector<gameobjects::Enemy> &blimps);
	void displayUfos(const std::vector<gameobjects::Enemy> &ufos);
	void displayPlanes(float totalTime, const std::vector<gameobjects::Enemy> &planes);
	void displayBullets(const std::vector<gameobjects::Bullet> &bullets);
	void displayMiniMapBackground();
	void displayEnemyMarkers(
		const std::vector<gameobjects::Enemy> &enemies,
		const game::Transform &playertransform
	);
	void displayCrosshair(const game::Transform &playertransform);
}

namespace gui {
	std::vector<std::string> readTextFile(const char *path);
	void displayFPSCounter(unsigned int fps);
	void displayHUD(unsigned int score, float speed, unsigned int health);
	//If the player got hit, then display a semi-transparent red background
	//on top of the screen to show that
	void displayDamage(float damagetimer);
	//Returns action taken by the user on the pause menu
	std::string displayPauseMenu();
	//Returns the action taken by the user on the death screen
	void displayDeathScreen(unsigned int finalscore);
	//returns the selected game mode
	game::GameMode displayMainMenu();
	//Display credits
	//returns true if user closed out of credits, false otherwise
	bool displayCredits(const std::vector<std::string> &credits);
	//Display High Scores
	bool displayHighScores(const HighScoreTable &highscores);
	//Display settings menu
	//returns the action taken by the user
	game::SettingsScreenAction displaySettingsMenu(SettingsValues &values);
}
