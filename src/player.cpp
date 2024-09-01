#include "game.hpp"
#include "app.hpp"
#include "audio.hpp"

constexpr unsigned int DEFAULT_HEALTH = 50;
constexpr float DAMAGE_COOLDOWN = 0.2f;
constexpr float DAMAGE_TIMER = 1.0f;
constexpr float ROTATION_Z_SPEED = 0.5f;
constexpr float MAX_ROTATION_Z = glm::radians(15.0f);
constexpr float ROTATION_Y_SPEED = 0.8f;
constexpr float ROTATION_X_SPEED = 0.6f;
constexpr float MAX_ROTATION_X = glm::radians(70.0f);

namespace gameobjects {
	Player::Player(glm::vec3 position) {
		transform.position = position;
		transform.rotation = glm::vec3(0.0f);
		transform.scale = glm::vec3(1.0f);
		xRotationDirection = RX_NONE;
		yRotationDirection = RY_NONE;
		crashed = false;
		speed = SPEED;
		health = DEFAULT_HEALTH;
	}

	void Player::damage(unsigned int amount) 
	{
		if(damagecooldown > 0.0f)
			return;

		if(health < amount)
			health = 0;
		else
			health -= amount;

		if(health > 0)
			SNDSRC->playid("hit", transform.position);

		damagecooldown = DAMAGE_COOLDOWN;
		damagetimer = DAMAGE_TIMER;
	}

	unsigned int Player::hpPercent() 
	{
		return (unsigned int)(float(health) / float(DEFAULT_HEALTH) * 100.0f);
	}

	float Player::damageTimerProgress()
	{
		return damagetimer / DAMAGE_TIMER;
	}

	void Player::rotateWithMouse(float dt)
	{
		State* state = State::get();
		double 
			dx = state->getMouseDX(),
			dy = state->getMouseDY();

		float
			speedx = 0.0f,
			speedy = ROTATION_Y_SPEED / 5.0f * dx;

		speedx = -ROTATION_X_SPEED * dy;

		speedx = std::min(speedx, ROTATION_X_SPEED / 2.0f);
		speedx = std::max(speedx, -ROTATION_X_SPEED / 2.0f);
		speedy = std::min(speedy, ROTATION_Y_SPEED / 3.0f);
		speedy = std::max(speedy, -ROTATION_Y_SPEED / 3.0f);

		if(yRotationDirection == Player::RY_NONE)
			transform.rotation.y -= speedy * dt;

		if(xRotationDirection == Player::RX_NONE) {
			transform.rotation.x -= speedx * dt;
			transform.rotation.x = std::min(transform.rotation.x, MAX_ROTATION_X);
			transform.rotation.x = std::max(transform.rotation.x, -MAX_ROTATION_X);
		}
	}

	void Player::update(float dt) 
	{
		//Keep track of how long the player has crashed,
		//this is so that the game can delay showing the death screen until
		//the explosion animation is done playing
		if(crashed) {
			deathtimer += dt;
			return;
		}

		//Shooting cooldown
		shoottimer -= dt;
		//Damage cooldown
		damagecooldown -= dt;
		damagetimer -= dt;

		State* state = State::get();	

		//Turn left/right
		if(state->getKeyState(GLFW_KEY_D) == JUST_PRESSED)	
			yRotationDirection = Player::RY_RIGHT;	
		else if(state->getKeyState(GLFW_KEY_A) == JUST_PRESSED)	
			yRotationDirection = Player::RY_LEFT;
		else if(state->getKeyState(GLFW_KEY_A) == RELEASED && 
				state->getKeyState(GLFW_KEY_D) == RELEASED)
			yRotationDirection = Player::RY_NONE;
	
		//Change pitch
		if(state->getKeyState(GLFW_KEY_S) == JUST_PRESSED)
			xRotationDirection = Player::RX_UP;
		else if(state->getKeyState(GLFW_KEY_W) == JUST_PRESSED)
			xRotationDirection = Player::RX_DOWN;
		else if(state->getKeyState(GLFW_KEY_S) == RELEASED &&
				state->getKeyState(GLFW_KEY_W) == RELEASED)
			xRotationDirection = Player::RX_NONE;

		//Rotate with mouse
		rotateWithMouse(dt);

		//Rotate on the y axis
		if(yRotationDirection == Player::RY_RIGHT) {
			transform.rotation.z += dt * ROTATION_Z_SPEED;
			transform.rotation.z =
				std::min(transform.rotation.z, MAX_ROTATION_Z);
			transform.rotation.y -= ROTATION_Y_SPEED * dt;
		}
		else if(yRotationDirection == Player::RY_LEFT) {
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

		//Rotate on the x axis
		if(xRotationDirection == Player::RX_UP) {
			transform.rotation.x -= dt * ROTATION_X_SPEED;
			transform.rotation.x =
				std::max(transform.rotation.x, -MAX_ROTATION_X);
		}
		else if(xRotationDirection == Player::RX_DOWN) {
			transform.rotation.x += dt * ROTATION_X_SPEED;
			transform.rotation.x =
				std::min(transform.rotation.x, MAX_ROTATION_X);
		}

		transform.position += transform.direction() * speed / 2.0f * dt;
		//Acceleration
		if(keyIsHeld(state->getKeyState(GLFW_KEY_LEFT_SHIFT)))
			speed += ACCELERATION * dt;
		else if(state->getScrollSpeed() > 0.0)
			speed += ACCELERATION * 4.0f * dt;
		else if(keyIsHeld(state->getKeyState(GLFW_KEY_LEFT_CONTROL)))
			speed -= ACCELERATION * dt;
		else if(state->getScrollSpeed() < 0.0)
			speed -= ACCELERATION * 4.0f * dt;
		speed = std::min(speed, SPEED * 3.0f);
		speed = std::max(speed, SPEED);
		transform.position += transform.direction() * speed / 2.0f * dt;
	}

	void Player::resetShootTimer()
	{
		shoottimer = 0.2f;
	}

	void Player::checkIfCrashed(float dt, const infworld::worldseed &permutations)
	{
		if(crashed)
			return;

		//Check if we ran out of health
		if(health <= 0) {
			crashed = true;
			return;
		}

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
}
