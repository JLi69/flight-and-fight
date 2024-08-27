#include "game.hpp"
#include "app.hpp"
#include <algorithm>

namespace gobjs = gameobjects;

constexpr float UFO_ROTATION_TIME = 10.0f;
//Plane rotation speed
constexpr float ROTATION_X = 0.3f;
constexpr float ROTATION_Y = 0.8f;
constexpr float ROTATION_Z = 0.5f;
constexpr float MAX_ROTATION_Z = glm::radians(15.0f);

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

	void Enemy::updatePlane(
		float dt,
		const Player &player,
		std::vector<Bullet> &bullets,
		const infworld::worldseed &permutations
	) {
		float h = infworld::getHeight(
			transform.position.z / SCALE * float(PREC + 1) / float(PREC),
			transform.position.x / SCALE * float(PREC + 1) / float(PREC),
			permutations
		) * HEIGHT * SCALE;
		float y = std::max(h, 0.0f);

		float speed = std::min(player.speed + 16.0f, 100.0f);
		glm::vec3 velocity = transform.direction() * speed;
		if(values.at("rotationdirection") < 0.0f)
			velocity *= 1.5f;
		transform.position += velocity * dt;
		values.at("rotationtimer") -= dt;
		//Difference between player position and object position 
		glm::vec3 diff = player.transform.position - transform.position;
		float dist = glm::length(diff);
		glm::vec3 dir = transform.direction();
		glm::vec2 
			diffxz = glm::normalize(glm::vec2(diff.x, diff.z)),
			dirxz = glm::normalize(glm::vec2(dir.x, dir.z));
		float dotprod = glm::dot(glm::normalize(diff), dir);

		if(dist < CHUNK_SZ * 2.0f && dotprod > 0.98f && values.at("rotationtimer") < 0.0f) {
			values.at("rotationdirection") = -3.0f;
			values.at("rotationtimer") = 6.0f;
		}
		else if(values.at("rotationtimer") < -30.0f && dist < CHUNK_SZ * 2.0f) {
			values.at("rotationdirection") = -3.0f;
			values.at("rotationtimer") = 6.0f;
		}
		else if(dotprod < -0.98f && values.at("rotationtimer") < 0.0f)
			values.at("rotationdirection") = 1.0f;
		float rotationdirection = values.at("rotationdirection");

		float rotationx = transform.rotation.x, rotationy = transform.rotation.y;
		float dotprod1, dotprod2;	
		
		transform.rotation.y += ROTATION_Y * dt;
		glm::vec3 dir1 = transform.direction();
		glm::vec2 dir1xz = glm::normalize(glm::vec2(dir1.x, dir1.z));
		dotprod1 = glm::dot(dir1xz, diffxz);
		transform.rotation.y = rotationy;	
		transform.rotation.y -= ROTATION_Y * dt;
		glm::vec3 dir2 = transform.direction();
		glm::vec2 dir2xz = glm::normalize(glm::vec2(dir2.x, dir2.z));
		dotprod2 = glm::dot(dir2xz, diffxz);
		transform.rotation.y = rotationy;

		if((!(dotprod < 0.0f && rotationdirection < 0.0f && dist > CHUNK_SZ) &&
			!(dotprod > 0.995f && rotationdirection > 0.0f))) {
			if(dotprod1 <= dotprod2) {
				transform.rotation.y -= ROTATION_Y * dt * rotationdirection;
				transform.rotation.z += ROTATION_Z * dt * rotationdirection;
			}
			else if(dotprod1 > dotprod2) {
				transform.rotation.y += ROTATION_Y * dt * rotationdirection;
				transform.rotation.z -= ROTATION_Z * dt * rotationdirection;
			}
		}
		else {
			if(transform.rotation.z > 0.0f)
				transform.rotation.z -= ROTATION_Z * dt;
			else if(transform.rotation.z < 0.0f)
				transform.rotation.z += ROTATION_Z * dt;
		}
		transform.rotation.z = std::max(transform.rotation.z, -MAX_ROTATION_Z);
		transform.rotation.z = std::min(transform.rotation.z, MAX_ROTATION_Z);

		transform.rotation.x += ROTATION_X * dt;
		dir1 = transform.direction();
		dotprod1 = glm::dot(dir1, diff);
		transform.rotation.x = rotationx;	
		transform.rotation.x -= ROTATION_X * dt;
		dir2 = transform.direction();
		dotprod2 = glm::dot(dir2, diff);
		transform.rotation.x = rotationx;
		
		if(transform.position.y - y <= 80.0f)
			transform.rotation.x -= ROTATION_X * 4.0f * dt;
		else if(values.at("rotationdirection") < 0.0f)
			transform.rotation.x -= transform.rotation.x * dt;
		else if(dotprod1 <= dotprod2 && values.at("rotationdirection") > 0.0f)
			transform.rotation.x -= ROTATION_X * dt;
		else if(dotprod1 > dotprod2 && values.at("rotationdirection") > 0.0f)
			transform.rotation.x += ROTATION_X * dt;

		transform.rotation.x = std::max(transform.rotation.x, -glm::radians(70.0f));
		transform.rotation.x = std::min(transform.rotation.x, glm::radians(70.0f));

		checkIfPlaneCrashed(permutations);
	}

	void Enemy::checkIfPlaneCrashed(const infworld::worldseed &permutations)
	{
		glm::vec3 pos = transform.position;
		float h = infworld::getHeight(
			pos.z / SCALE * float(PREC + 1) / float(PREC),
			pos.x / SCALE * float(PREC + 1) / float(PREC),
			permutations
		) * HEIGHT * SCALE;
		//Maximum height difference between
		const float MAX_HEIGHT_DIFF = 8.0f;
		if(pos.y - h < MAX_HEIGHT_DIFF || pos.y < MAX_HEIGHT_DIFF / 2.0f) {
			//Do not give any score if the plane crashes into the terrain
			scorevalue = 0;
			hitpoints = 0;
			return;
		}

		glm::vec3 positions[] = {
			transform.position + transform.rotate(glm::vec3(-9.0f, 0.0f, 0.0f)),
			transform.position + transform.rotate(glm::vec3(10.0f, 0.0f, 0.0f)),
			transform.position + transform.rotate(glm::vec3(-13.0f, -5.0f, 0.0f)),
			transform.position + transform.rotate(glm::vec3(13.0f, -5.0f, 0.0f)),
		};

		for(int i = 0; i < 4; i++) {
			glm::vec3 pos = positions[i];
			h = infworld::getHeight(
				pos.z / SCALE * float(PREC + 1) / float(PREC),
				pos.x / SCALE * float(PREC + 1) / float(PREC),
				permutations
			) * HEIGHT * SCALE;
			if(pos.y - h < MAX_HEIGHT_DIFF || pos.y < MAX_HEIGHT_DIFF / 2.0f) {
				scorevalue = 0;
				hitpoints = 0;
				return;
			}
		}
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

	Enemy spawnBalloon(const glm::vec3 &position, const infworld::worldseed &permutations)
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
		glm::vec3 pos(position.x, HEIGHT * SCALE * 1.1f, position.z);
		Enemy blimp = Enemy(pos, 24, 30);
		blimp.transform.rotation.y = rotation;
		return blimp;
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

	Enemy spawnPlane(
		const glm::vec3 &position,
		float rotation,
		const infworld::worldseed &permutations
	) {
		float h = infworld::getHeight(
			position.z / SCALE * float(PREC + 1) / float(PREC),
			position.x / SCALE * float(PREC + 1) / float(PREC),
			permutations
		) * HEIGHT * SCALE;
		float y = std::max(h, 0.0f) + HEIGHT;
		glm::vec3 pos(position.x, y, position.z);

		Enemy plane = Enemy(pos, 12, 50);
		plane.setVal("rotationdirection", 1.0f);
		plane.setVal("rotationtimer", 0.0f);
		plane.transform.rotation.y = rotation;
		return plane;
	}
}

namespace game {
	void spawnBalloons(
		gobjs::Player &player,
		std::vector<gobjs::Enemy> &balloons,
		std::minstd_rand0 &lcg,
		const infworld::worldseed &permutations
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

	void spawnPlanes(
		gameobjects::Player &player,
		std::vector<gameobjects::Enemy> &planes,
		std::minstd_rand0 &lcg,
		const infworld::worldseed &permutations
	) {
		if(planes.size() >= 6)
			return;

		int randval = lcg() % 3;
		if(randval == 0 && planes.size() > 2)
			return;
	
		glm::vec3 center = player.transform.position;
		float dist = float(lcg() % 256) / 256.0f * CHUNK_SZ * 12.0f + CHUNK_SZ * 6.0f;
		float angle = float(lcg() % 256) / 256.0f * glm::radians(360.0f);
		glm::vec3 position = center + dist * glm::vec3(cosf(angle), 0.0f, sinf(angle));
		float rotation = float(lcg() % 256) / 256.0f * glm::radians(360.0f);
		planes.push_back(gobjs::spawnPlane(position, rotation, permutations));
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

				if(glm::length(diff) < crashdist && !player.crashed) {
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
