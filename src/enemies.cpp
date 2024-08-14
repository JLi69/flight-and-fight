#include "game.hpp"
#include "app.hpp"
#include <algorithm>

namespace gobjs = gameobjects;

namespace gameobjects {
	Enemy::Enemy(glm::vec3 position, int hp, unsigned int scoreval)
	{
		transform.position = position;
		transform.scale = glm::vec3(1.0f);
		transform.rotation = glm::vec3(0.0f);
		hitpoints = hp;
		scorevalue = scoreval;
	}

	void Enemy::updateBalloon(float dt)
	{
		if(transform.position.y > values.at("maxy")) {
			transform.position.y = values.at("maxy");
			values.at("direction") *= -1.0f;
		}
		else if(transform.position.y < values.at("miny")) {
			transform.position.y = values.at("miny");
			values.at("direction") *= -1.0f;
		}

		transform.position.y += 16.0f * dt * values.at("direction");
	}

	float Enemy::getVal(const std::string &key)
	{
		return values[key];
	}

	void Enemy::setVal(const std::string &key, float v)
	{
		values[key] = v;
	}
}

namespace game {
	void spawnBalloons(
		gobjs::Player &player,
		std::vector<gobjs::Enemy> &balloons,
		std::minstd_rand0 &lcg,
		infworld::worldseed &permutations
	) {
		if(balloons.size() >= 4)
			return;

		int randval = lcg() % 2;
		if(randval > 0 && balloons.size() > 1)
			return;

		glm::vec3 center = player.transform.position;
		float dist = float(lcg() % 256) / 256.0f * CHUNK_SZ * 12.0f + CHUNK_SZ * 6.0f;
		float angle = float(lcg() % 256) / 256.0f * glm::radians(360.0f);
		glm::vec3 position = center + dist * glm::vec3(cosf(angle), 0.0f, sinf(angle));
		balloons.push_back(gobjs::spawnBalloon(position, permutations));
	}

	void destroyEnemies(
		gobjs::Player &player,
		std::vector<gobjs::Enemy> &enemies,
		std::vector<gobjs::Explosion> &explosions,
		float crashdist,
		unsigned int &score
	) {
		if(enemies.empty())
			return;

		enemies.erase(std::remove_if(
			enemies.begin(),
			enemies.end(),
			[&player, &explosions, &crashdist, &score](gobjs::Enemy &enemy) {
				if(enemy.hitpoints <= 0) {
					explosions.push_back(gobjs::Explosion(enemy.transform.position));
					score += enemy.scorevalue;
					return true;
				}

				glm::vec3 diff = enemy.transform.position - player.transform.position;

				if(glm::length(diff) < crashdist) {
					player.crashed = true;
					explosions.push_back(gobjs::Explosion(enemy.transform.position));
					return true;
				}

				return glm::length(diff) > CHUNK_SZ * 32.0f;
			}
		), enemies.end());
	}
}
