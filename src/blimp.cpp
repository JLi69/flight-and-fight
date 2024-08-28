#include "game.hpp"

namespace gobjs = gameobjects;

namespace gameobjects {
	void Enemy::updateBlimp(float dt)
	{
		transform.position += transform.direction() * 24.0f * dt;
	}

	Enemy spawnBlimp(const glm::vec3 &position, float rotation)
	{
		glm::vec3 pos(position.x, HEIGHT * SCALE * 1.1f, position.z);
		Enemy blimp = Enemy(pos, 24, 60);
		blimp.transform.rotation.y = rotation;
		return blimp;
	}
}

namespace game {
	void spawnBlimps(
		gobjs::Player &player,
		std::vector<gobjs::Enemy> &blimps,
		std::minstd_rand0 &lcg
	) {
		if(blimps.size() >= 3)
			return;

		int randval = lcg() % 2;
		if(randval > 0)
			return;

		glm::vec3 center = player.transform.position;
		float dist = float(lcg() % 256) / 256.0f * CHUNK_SZ * 12.0f + CHUNK_SZ * 6.0f;
		float angle = float(lcg() % 256) / 256.0f * glm::radians(360.0f);
		glm::vec3 position = center + dist * glm::vec3(cosf(angle), 0.0f, sinf(angle));
		float rotation = float(lcg() % 256) / 256.0f * glm::radians(360.0f);
		gobjs::Enemy blimp = gobjs::spawnBlimp(position, rotation);
		blimp.transform.position.y += float(lcg() % 256) / 256.0f * 64.0f;
		blimps.push_back(blimp);
	}
}
