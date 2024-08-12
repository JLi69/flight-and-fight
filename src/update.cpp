#include "game.hpp"
#include "app.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

constexpr float ROTATION_Z_SPEED = 0.5f;
constexpr float MAX_ROTATION_Z = glm::radians(15.0f);
constexpr float ROTATION_Y_SPEED = 0.7f;
constexpr float ROTATION_X_SPEED = 0.6f;
constexpr float MAX_ROTATION_X = glm::radians(50.0f);

namespace gobjs = gameobjects;

namespace gameobjects {
	Player::Player(glm::vec3 position) {
		transform.position = position;
		transform.rotation = glm::vec3(0.0f);
		transform.scale = glm::vec3(1.0f);
		xRotationDirection = RX_NONE;
		yRotationDirection = RY_NONE;
		crashed = false;
	}

	void Player::update(float dt) 
	{
		if(crashed) {
			deathtimer += dt;
			return;
		}

		shoottimer -= dt;

		State* state = State::get();	

		if(state->getKeyState(GLFW_KEY_D) == JUST_PRESSED)	
			yRotationDirection = gobjs::Player::RY_RIGHT;	
		else if(state->getKeyState(GLFW_KEY_A) == JUST_PRESSED)	
			yRotationDirection = gobjs::Player::RY_LEFT;
		else if(state->getKeyState(GLFW_KEY_A) == RELEASED && 
				state->getKeyState(GLFW_KEY_D) == RELEASED)
			yRotationDirection = gobjs::Player::RY_NONE;			

		if(yRotationDirection == gobjs::Player::RY_RIGHT) {
			transform.rotation.z += dt * ROTATION_Z_SPEED;
			transform.rotation.z =
				std::min(transform.rotation.z, MAX_ROTATION_Z);
			transform.rotation.y -= ROTATION_Y_SPEED * dt;
		}
		else if(yRotationDirection == gobjs::Player::RY_LEFT) {
			transform.rotation.z -= dt * ROTATION_Z_SPEED;
			transform.rotation.z =
				std::max(transform.rotation.z, -MAX_ROTATION_Z);
			transform.rotation.y += ROTATION_Y_SPEED * dt;
		}
		else {
			if(transform.rotation.z < 0.0f)
				transform.rotation.z += dt * ROTATION_Z_SPEED / 2.0f;
			else if(transform.rotation.z > 0.0f)
				transform.rotation.z -= dt * ROTATION_Z_SPEED / 2.0f;
		
			if(std::abs(transform.rotation.z) < glm::radians(0.5f))
				transform.rotation.z = 0.0f;
		}
		
		if(state->getKeyState(GLFW_KEY_W) == JUST_PRESSED)
			xRotationDirection = gobjs::Player::RX_UP;
		else if(state->getKeyState(GLFW_KEY_S) == JUST_PRESSED)
			xRotationDirection = gobjs::Player::RX_DOWN;
		else if(state->getKeyState(GLFW_KEY_S) == RELEASED &&
				state->getKeyState(GLFW_KEY_W) == RELEASED)
			xRotationDirection = gobjs::Player::RX_NONE;

		if(xRotationDirection == gobjs::Player::RX_UP) {
			transform.rotation.x -= dt * ROTATION_X_SPEED;
			transform.rotation.x =
				std::max(transform.rotation.x, -MAX_ROTATION_X);
		}
		else if(xRotationDirection == gobjs::Player::RX_DOWN) {
			transform.rotation.x += dt * ROTATION_X_SPEED;
			transform.rotation.x =
				std::min(transform.rotation.x, MAX_ROTATION_X);
		}

		transform.position += transform.direction() * SPEED * dt;
	}

	void Player::resetShootTimer()
	{
		shoottimer = 0.2f;
	}

	void Player::checkIfCrashed(float dt, infworld::worldseed &permutations)
	{
		if(crashed)
			return;

		glm::vec3 pos = transform.position + transform.direction() * SPEED * dt;
		float h = infworld::getHeight(
			pos.z / SCALE * float(PREC + 1) / float(PREC),
			pos.x / SCALE * float(PREC + 1) / float(PREC),
			permutations
		) * HEIGHT * SCALE;
		//Maximum height difference between
		const float MAX_HEIGHT_DIFF = 8.0f;
		if(pos.y - h < MAX_HEIGHT_DIFF || pos.y < MAX_HEIGHT_DIFF / 2.0f) {
			crashed = true;
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
				crashed = true;
				return;
			}
		}
	}

	Explosion::Explosion(glm::vec3 position)
	{
		transform.position = position;
		transform.scale = glm::vec3(1.0f);
		transform.rotation = glm::vec3(0.0f);
		timePassed = 0.0f;
		visible = true;
	}

	void Explosion::update(float dt)
	{
		if(!visible)
			return;
		
		timePassed += dt;
		transform.scale = glm::vec3(10.0f) * timePassed + glm::vec3(1.0f);

		if(timePassed > 2.0f)
			visible = false;
	}

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

	Bullet::Bullet(const Player &player, const glm::vec3 &offset)
	{
		time = 0.0f;
		transform.position = 
			player.transform.rotate(offset) +
			player.transform.position;
		transform.rotation = player.transform.rotation;
		transform.scale = glm::vec3(1.0f);
	}

	void Bullet::update(float dt)
	{
		time += dt;
		transform.position += transform.direction() * dt * BULLET_SPEED;
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
}

namespace game {
	glm::vec3 getCameraFollowPos(const Transform &playertransform)
	{
		glm::mat4 transformMat(1.0f);
		transformMat = glm::translate(transformMat, playertransform.position);	
		transformMat = glm::rotate(
			transformMat, 
			playertransform.rotation.z / 3.0f,
			glm::vec3(0.0f, 0.0f, 1.0f)
		);
		transformMat = glm::rotate(
			transformMat, 
			playertransform.rotation.y,
			glm::vec3(0.0f, 1.0f, 0.0f)
		);	
		transformMat = glm::rotate(
			transformMat, 
			playertransform.rotation.x / 2.0f,
			glm::vec3(1.0f, 0.0f, 0.0f)
		);

		float yoffset = (playertransform.rotation.x - MAX_ROTATION_X) / (2.0f * MAX_ROTATION_X) * 36.0f;
		const float dist = sqrtf(48.0f * 48.0f + 40.0f * 40.0f);
		float y = 48.0f + yoffset;
		float z = -sqrtf(dist * dist - y * y);
		glm::vec3 cameraOffset = glm::vec3(0.0f, y, z);
		glm::mat4 cameraTransform =
			glm::translate(glm::mat4(1.0f), cameraOffset);

		return glm::vec3(transformMat * cameraTransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	void updateCamera(gobjs::Player &player)
	{
		Camera& cam = State::get()->getCamera();
		//Update camera
		cam.position = game::getCameraFollowPos(player.transform);
		cam.yaw = -(player.transform.rotation.y + glm::radians(180.0f));
		cam.pitch = glm::radians(25.0f) + player.transform.rotation.x * 0.6f;
	}

	void spawnBalloons(
		gameobjects::Player &player,
		std::vector<gameobjects::Enemy> &balloons,
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
		gameobjects::Player &player,
		std::vector<gobjs::Enemy> &enemies,
		std::vector<gameobjects::Explosion> &explosions,
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

	void updateExplosions(
		std::vector<gobjs::Explosion> &explosions, 
		const glm::vec3 &center,
		float dt
	) {
		for(auto &explosion : explosions)
			explosion.update(dt);

		explosions.erase(std::remove_if(
			explosions.begin(),
			explosions.end(),
			[](gobjs::Explosion &explosion) {
				return !explosion.visible;
			}
		), explosions.end());

		std::sort(
			explosions.begin(),
			explosions.end(),
			[&center](gobjs::Explosion &e1, gobjs::Explosion &e2) {
				glm::vec3 
					d1 = e1.transform.position - center,
					d2 = e2.transform.position - center;
				return glm::length(d1) < glm::length(d2);
			}
		);
	}

	void updateBullets(std::vector<gameobjects::Bullet> &bullets, float dt)
	{
		for(auto &bullet : bullets)
			bullet.update(dt);
	
		bullets.erase(std::remove_if(
			bullets.begin(),
			bullets.end(),
			[](gobjs::Bullet &bullet) {
				return bullet.time > 2.0f || bullet.destroyed || bullet.transform.position.y < 0.0f;
			}
		), bullets.end());
	}

	void checkForHit(
		std::vector<gameobjects::Bullet> &bullets,
		std::vector<gameobjects::Enemy> &enemies,
		float hitdist
	) {
		for(auto &bullet : bullets) {
			for(auto &enemy : enemies) {
				glm::vec3 diff = bullet.transform.position - enemy.transform.position;
				float dist = glm::length(diff);
				if(dist < hitdist) {
					bullet.destroyed = true;
					enemy.hitpoints--;
				}
			}
		}
	}

	void checkForBulletTerrainCollision(
		std::vector<gameobjects::Bullet> &bullets,
		infworld::worldseed &permutations
	) {
		bullets.erase(std::remove_if(
			bullets.begin(),
			bullets.end(),
			[&permutations](gobjs::Bullet &bullet) {
				glm::vec3 pos = bullet.transform.position;
				float h = infworld::getHeight(
					pos.z / SCALE * float(PREC + 1) / float(PREC),
					pos.x / SCALE * float(PREC + 1) / float(PREC),
					permutations
				) * HEIGHT * SCALE;
				return pos.y < h;
			}
		), bullets.end());
	}
}
