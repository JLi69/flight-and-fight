#include "game.hpp"
#include "app.hpp"
#include "audio.hpp"
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
}

namespace game {
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
					SNDSRC->playid("explosion", enemy.transform.position);
					explosions.push_back(gobjs::Explosion(enemy.transform.position, explosionscale));
					score += enemy.scorevalue;
					return true;
				}

				glm::vec3 diff = enemy.transform.position - player.transform.position;

				if(glm::length(diff) < crashdist && !player.crashed) {
					SNDSRC->playid(
						"explosion",
						enemy.transform.position,
						explosionscale
					);
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

	void checkForCollision(std::vector<gameobjects::Enemy> &enemies, float hitdist)
	{
		for(int i = 0; i < enemies.size(); i++) {
			for(int j = i + 1; j < enemies.size(); j++) {
				glm::vec3
					p1 = enemies.at(i).transform.position,
					p2 = enemies.at(j).transform.position;
				float dist = glm::length(p1 - p2);

				if(dist < hitdist) {
					enemies.at(i).scorevalue = 0;
					enemies.at(i).hitpoints = 0;
					enemies.at(j).scorevalue = 0;
					enemies.at(j).hitpoints = 0;
				}
			}
		}
	}
}
