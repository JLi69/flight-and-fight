#include "game.hpp"

namespace gobjs = gameobjects;

constexpr float UFO_ROTATION_TIME = 10.0f;

namespace gameobjects {
	void Enemy::updateUfo(float dt, const infworld::worldseed &permutations)
	{
		transform.position += transform.direction() * 144.0f * dt;
		float h = infworld::getHeight(
			transform.position.z / SCALE * float(PREC + 1) / float(PREC),
			transform.position.x / SCALE * float(PREC + 1) / float(PREC),
			permutations
		) * HEIGHT * SCALE;
		float y = std::max(h, 0.0f) + HEIGHT * 1.25f;
		if(std::abs(transform.position.y - y) > 4.0f)
			transform.position.y += (y - transform.position.y) * 2.0f * dt;

		values.at("rotationtimer") -= dt;
		if(values.at("rotationtimer") < 0.0f && values.at("rotationtimer") > -1.0f)
			transform.rotation.y += values.at("rotationspeed") * dt;

		if(values.at("rotationtimer") <= -1.0f) {
			values.at("rotationspeed") *= -1.0f;
			values.at("rotationtimer") = UFO_ROTATION_TIME;
		}
	}

	Enemy spawnUfo(
		const glm::vec3 &position,
		float rotation,
		const infworld::worldseed &permutations
	) {
		float h = infworld::getHeight(
			position.z / SCALE * float(PREC + 1) / float(PREC),
			position.x / SCALE * float(PREC + 1) / float(PREC),
			permutations
		) * HEIGHT * SCALE;
		float y = std::max(h, 0.0f) + HEIGHT * 1.25f;
		glm::vec3 pos(position.x, y, position.z);

		Enemy ufo = Enemy(pos, 8, 500);
		ufo.transform.rotation.y = rotation;
		ufo.setVal("rotationspeed", 1.0f);
		ufo.setVal("rotationtimer", UFO_ROTATION_TIME);
		return ufo;
	}
}

namespace game {
	void spawnUfos(
		gobjs::Player &player,
		std::vector<gobjs::Enemy> &ufos,
		std::minstd_rand0 &lcg,
		const infworld::worldseed &permutations
	) {
		if(ufos.size() >= 2)
			return;

		int randval = lcg() % 3;
		if(randval > 0)
			return;

		glm::vec3 center = player.transform.position;
		float dist = float(lcg() % 256) / 256.0f * CHUNK_SZ * 12.0f + CHUNK_SZ * 6.0f;
		float angle = float(lcg() % 256) / 256.0f * glm::radians(360.0f);
		glm::vec3 position = center + dist * glm::vec3(cosf(angle), 0.0f, sinf(angle));
		float rotation = float(lcg() % 256) / 256.0f * glm::radians(360.0f);
		ufos.push_back(gobjs::spawnUfo(position, rotation, permutations));
	}
}
