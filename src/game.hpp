#pragma once

#include "infworld.hpp"

//Constants
constexpr float SPEED = 64.0f;
constexpr unsigned int MAX_LOD = 5;
constexpr float LOD_SCALE = 2.0f;

constexpr float FOVY = glm::radians(75.0f);
constexpr float ZNEAR = 2.0f;
constexpr float ZFAR = 20000.0f;

constexpr int RANGE = 8;

const glm::vec3 LIGHT = glm::normalize(glm::vec3(-1.0f));

namespace game {
	struct Transform {
		glm::vec3 position;
		glm::vec3 scale;
		glm::vec3 rotation; //in radians
		glm::mat4 getTransformMat() const;
		glm::vec3 direction() const;
	};	

	void loadAssets();
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
}

namespace gameobjects {
	struct Player {
		game::Transform transform;
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
		Player(glm::vec3 position);
	};
}

namespace game {
	//Returns the position the camera should be following
	glm::vec3 getCameraFollowPos(const Transform &playertransform);
	//Have the camera follow the player
	void updateCamera(gameobjects::Player &player);
	void updatePlayer(gameobjects::Player &player, float dt);
}

namespace gfx {
	void displaySkybox();
	void displayWater(float totalTime);
	void displayDecorations(infworld::DecorationTable &decorations, float totalTime);
	unsigned int displayTerrain(infworld::ChunkTable *chunktables, int maxlod, float lodscale);	
	void generateDecorationOffsets(infworld::DecorationTable &decorations);
	void displayPlayerPlane(float totalTime, const game::Transform &transform);
}
