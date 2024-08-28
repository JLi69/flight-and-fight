#include "game.hpp"
#include "app.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace gobjs = gameobjects;

namespace gameobjects {	
	Explosion::Explosion(glm::vec3 position)
	{
		transform.position = position;
		transform.scale = glm::vec3(1.0f);
		transform.rotation = glm::vec3(0.0f);
		timePassed = 0.0f;
		explosionScale = 1.0f;
		visible = true;
	}

	Explosion::Explosion(glm::vec3 position, float scale)
	{
		transform.position = position;
		transform.scale = glm::vec3(1.0f);
		explosionScale = scale;
		transform.rotation = glm::vec3(0.0f);
		timePassed = 0.0f;
		visible = true;
	}

	void Explosion::update(float dt)
	{
		if(!visible)
			return;
		
		timePassed += dt;
		transform.scale = (glm::vec3(10.0f) * timePassed + glm::vec3(1.0f)) * explosionScale;

		if(timePassed > 2.0f)
			visible = false;
	}	

	Bullet::Bullet(const Player &player, const glm::vec3 &offset)
	{
		time = 0.0f;
		transform.position = 
			player.transform.rotate(offset) +
			player.transform.position;
		transform.rotation = player.transform.rotation;
		transform.scale = glm::vec3(1.0f);
		speed = BULLET_SPEED + player.speed - SPEED;
	}

	Bullet::Bullet(const game::Transform &t, float addspeed, const glm::vec3 &offset)
	{
		time = 0.0f;
		transform.position = 
			t.rotate(offset) +
			t.position;
		transform.rotation = t.rotation;
		transform.scale = glm::vec3(1.0f);
		speed = BULLET_SPEED + addspeed - SPEED;
	}

	Bullet::Bullet()
	{
		time = 0.0f;
		speed = BULLET_SPEED;
	}

	void Bullet::update(float dt)
	{
		time += dt;
		transform.position += transform.direction() * dt * speed;
	}	
}

namespace game {
	glm::vec3 getCameraFollowPos(const Transform &playertransform)
	{
		glm::vec3 offset = playertransform.rotate(glm::vec3(0.0f, 28.0f, -60.0f));
		return playertransform.position + offset;
	}

	void updateCamera(gobjs::Player &player)
	{
		Camera& cam = State::get()->getCamera();
		//Update camera
		cam.position = game::getCameraFollowPos(player.transform);
		cam.yaw = -(player.transform.rotation.y + glm::radians(180.0f));
		cam.pitch = player.transform.rotation.x;
	}

	void updateCamera(gobjs::Player &player, float dt)
	{
		Camera& cam = State::get()->getCamera();
		//Update camera
		cam.position = game::getCameraFollowPos(player.transform);
		float
			yaw = -(player.transform.rotation.y + glm::radians(180.0f)),
			pitch = player.transform.rotation.x;
		cam.yaw += (yaw - cam.yaw) * 6.0f * dt;
		cam.pitch += (pitch - cam.pitch) * 7.0f * dt;
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
				return glm::length(d1) > glm::length(d2);
			}
		);
	}

	void updateBullets(std::vector<gobjs::Bullet> &bullets, float dt)
	{
		for(auto &bullet : bullets)
			bullet.update(dt);
	
		bullets.erase(std::remove_if(
			bullets.begin(),
			bullets.end(),
			[](gobjs::Bullet &bullet) {
				return bullet.destroyed || bullet.transform.position.y < 0.0f;
			}
		), bullets.end());
	}

	void checkBulletDist(
		std::vector<gobjs::Bullet> &bullets,
		const gobjs::Player &player
	) {
		for(auto &bullet : bullets) {
			glm::vec3 diff = bullet.transform.position - player.transform.position;
			float dist = glm::length(diff);
			if(dist > BULLET_SPEED * 3.0f)
				bullet.destroyed = true;
		}
	}

	void checkForHit(
		std::vector<gobjs::Bullet> &bullets,
		std::vector<gobjs::Enemy> &enemies,
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

	void checkForHit(
		std::vector<gobjs::Bullet> &bullets,
		gobjs::Player &player,
		float hitdist
	) {
		for(auto &bullet : bullets) {
			glm::vec3 diff = bullet.transform.position - player.transform.position;
			float dist = glm::length(diff);
			if(dist < hitdist) {
				bullet.destroyed = true;
				player.damage(1);
			}
		}
	}

	void checkForBulletTerrainCollision(
		std::vector<gobjs::Bullet> &bullets,
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
