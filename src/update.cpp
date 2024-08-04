#include "game.hpp"
#include "app.hpp"
#include <glm/gtc/matrix_transform.hpp>

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
}
