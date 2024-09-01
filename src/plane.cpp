#include "game.hpp"
#include "audio.hpp"

namespace gobjs = gameobjects;

//Plane rotation speed
constexpr float ROTATION_X = 0.3f;
constexpr float ROTATION_Y = 0.8f;
constexpr float ROTATION_Z = 0.5f;
constexpr float MAX_ROTATION_Z = glm::radians(15.0f);
constexpr float SHOOT_COOLDOWN = 0.5f;

namespace gameobjects {
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
		values.at("shoottimer") -= dt;
		//Difference between player position and object position 
		glm::vec3 diff = player.transform.position - transform.position;
		float dist = glm::length(diff);
		glm::vec3 dir = transform.direction();
		glm::vec2 
			diffxz = glm::normalize(glm::vec2(diff.x, diff.z)),
			dirxz = glm::normalize(glm::vec2(dir.x, dir.z));
		float dotprod = glm::dot(glm::normalize(diff), dir);

		//Shoot
		if(values.at("shoottimer") < 0.0f && 
			dotprod > 0.8f && 
			values.at("rotationdirection") > 0.0f &&
			dist < CHUNK_SZ * 12.0f &&
			!player.crashed) {
			SNDSRC->playid("shoot", transform.position, 2.0f);
			values.at("shoottimer") = SHOOT_COOLDOWN;
			bullets.push_back(gobjs::Bullet(transform, speed, glm::vec3(-8.5f, -0.75f, 8.5f)));
			bullets.push_back(gobjs::Bullet(transform, speed, glm::vec3(8.5f, -0.75f, 8.5f)));
		}

		if(dist < CHUNK_SZ * 2.0f && dotprod > 0.98f && values.at("rotationtimer") < 0.0f
			|| dist < CHUNK_SZ / 2.0f) {
			values.at("rotationdirection") = -3.0f;
			values.at("rotationtimer") = 6.0f;
		}
		else if(values.at("rotationtimer") < -30.0f && dist < CHUNK_SZ * 2.0f) {
			values.at("rotationdirection") = -3.0f;
			values.at("rotationtimer") = 6.0f;
		}
		else if(dotprod < -0.98f && values.at("rotationtimer") < 0.0f)
			values.at("rotationdirection") = 1.0f;
		else if(dist > CHUNK_SZ * 12.0f && values.at("rotationtimer") < 0.0f)
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
		else if(values.at("rotationdirection") < 0.0f && dist > CHUNK_SZ) {
			transform.rotation.x -= transform.rotation.x * dt;
			if(std::abs(transform.rotation.x) < 0.02f)
				transform.rotation.x = 0.0f;
		}
		else if(dist <= CHUNK_SZ) {
			if(dotprod1 <= dotprod2)
				transform.rotation.x += ROTATION_X * dt * 2.0f;
			else if(dotprod1 > dotprod2)
				transform.rotation.x -= ROTATION_X * dt * 2.0f;
		}
		else if(dotprod1 <= dotprod2 && values.at("rotationdirection") > 0.0f && dotprod < 0.995f)
			transform.rotation.x -= ROTATION_X * dt;
		else if(dotprod1 > dotprod2 && values.at("rotationdirection") > 0.0f && dotprod < 0.995f)
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

		Enemy plane = Enemy(pos, 14, 50);
		plane.setVal("rotationdirection", 1.0f);
		plane.setVal("rotationtimer", 0.0f);
		plane.setVal("shoottimer", 0.0f);
		plane.transform.rotation.y = rotation;
		return plane;
	}
}

namespace game {
	void spawnPlanes(
		gameobjects::Player &player,
		std::vector<gameobjects::Enemy> &planes,
		std::minstd_rand0 &lcg,
		const infworld::worldseed &permutations,
		float totalTime
	) {
		if(planes.size() >= 6)
			return;

		int randval = lcg() % 3;
		if(randval == 0 && planes.size() > 2)
			return;

		unsigned int mincount = 1, maxcount = 1;
		if(totalTime < 90.0f) {
			mincount = 1;
			maxcount = 1;
		}
		else if(totalTime < 180.0f) {
			mincount = 1;
			maxcount = 2;
		}
		else {
			mincount = 2;
			maxcount = 4;
		}

		unsigned int count = lcg() % (maxcount - mincount + 1) + mincount;
		for(int i = 0; i < count; i++) {
			glm::vec3 center = player.transform.position;
			float dist = float(lcg() % 256) / 256.0f * CHUNK_SZ * 12.0f + CHUNK_SZ * 6.0f;
			float angle = float(lcg() % 256) / 256.0f * glm::radians(360.0f);
			glm::vec3 position = center + dist * glm::vec3(cosf(angle), 0.0f, sinf(angle));
			float rotation = float(lcg() % 256) / 256.0f * glm::radians(360.0f);
			planes.push_back(gobjs::spawnPlane(position, rotation, permutations));
		}
	}
}
