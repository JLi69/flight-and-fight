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

	void Enemy::updateBlimp(float dt)
	{
		transform.position += transform.direction() * 24.0f * dt;
	}

	float Enemy::getVal(const std::string &key) const
	{
		if(!values.count(key))
			return 0.0f;
		return values.at(key);
	}

	void Enemy::setVal(const std::string &key, float v)
	{
		values[key] = v;
	}

	Enemy spawnBalloon(const glm::vec3 &position, infworld::worldseed &permutations)
	{
		float h = infworld::getHeight(
			position.z / SCALE * float(PREC + 1) / float(PREC),
			position.x / SCALE * float(PREC + 1) / float(PREC),
			permutations
		) * HEIGHT * SCALE;
		float y = std::max(h, 0.0f) + HEIGHT;
		glm::vec3 pos(position.x, y, position.z);
		Enemy balloon = Enemy(pos, 5, 10);

		float miny = y;
		float maxy = y + HEIGHT;
		float direction = 1.0f;	
		balloon.setVal("miny", miny);
		balloon.setVal("maxy", maxy);
		balloon.setVal("direction", direction);
		return balloon;
	}

	Enemy spawnBlimp(const glm::vec3 &position, float rotation)
	{
		glm::vec3 pos(position.x, HEIGHT * SCALE, position.z);
		Enemy blimp = Enemy(pos, 16, 30);
		blimp.transform.rotation.y = rotation;
		return blimp;
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

	void spawnBlimps(
		gobjs::Player &player,
		std::vector<gobjs::Enemy> &blimps,
		std::minstd_rand0 &lcg,
		infworld::worldseed &permutations
	) {
		if(blimps.size() >= 2)
			return;

		int randval = lcg() % 2;
		if(randval > 0)
			return;

		glm::vec3 center = player.transform.position;
		float dist = float(lcg() % 256) / 256.0f * CHUNK_SZ * 12.0f + CHUNK_SZ * 6.0f;
		float angle = float(lcg() % 256) / 256.0f * glm::radians(360.0f);
		glm::vec3 position = center + dist * glm::vec3(cosf(angle), 0.0f, sinf(angle));
		float rotation = float(lcg() % 256) / 256.0f * glm::radians(360.0f);
		blimps.push_back(gobjs::spawnBlimp(position, rotation));
	}

	void destroyEnemies(
		gobjs::Player &player,
		std::vector<gobjs::Enemy> &enemies,
		std::vector<gobjs::Explosion> &explosions,
		float explosionscale,
		float crashdist,
		unsigned int &score
	) {
		if(enemies.empty())
			return;

		enemies.erase(std::remove_if(
			enemies.begin(),
			enemies.end(),
			[&player, &explosions, &crashdist, &score, &explosionscale](gobjs::Enemy &enemy) {
				if(enemy.hitpoints <= 0) {
					explosions.push_back(gobjs::Explosion(enemy.transform.position, explosionscale));
					score += enemy.scorevalue;
					return true;
				}

				glm::vec3 diff = enemy.transform.position - player.transform.position;

				if(glm::length(diff) < crashdist) {
					player.crashed = true;
					explosions.push_back(gobjs::Explosion(enemy.transform.position, explosionscale));
					return true;
				}

				return glm::length(diff) > CHUNK_SZ * 32.0f;
			}
		), enemies.end());
	}

	void checkForCollision(
		gameobjects::Player &player,
		std::vector<gobjs::Enemy> &enemies,
		std::vector<gobjs::Explosion> &explosions,
		float explosionscale,
		const glm::vec3 &extents
	) {
		if(enemies.empty())
			return;

		enemies.erase(std::remove_if(
			enemies.begin(),
			enemies.end(),
			[&player, &explosions, &extents, &explosionscale](gobjs::Enemy &enemy) {
				glm::vec3 diff = player.transform.position - enemy.transform.position;
				diff = enemy.transform.invRotate(diff);

				if(std::abs(diff.x) < extents.x && 
					std::abs(diff.y) < extents.y &&
					std::abs(diff.z) < extents.z) {
					player.crashed = true;
					explosions.push_back(gobjs::Explosion(enemy.transform.position, explosionscale));
					return true;
				}

				return false;
			}
		), enemies.end());
	}	
}
