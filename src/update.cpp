#include "game.hpp"
#include "app.hpp"
#include <glm/gtc/matrix_transform.hpp>

constexpr float ROTATION_Z_SPEED = 0.5f;
constexpr float MAX_ROTATION_Z = glm::radians(15.0f);
constexpr float ROTATION_Y_SPEED = 0.7f;
constexpr float ROTATION_X_SPEED = 0.6f;
constexpr float MAX_ROTATION_X = glm::radians(40.0f);

namespace gobjs = gameobjects;

namespace gameobjects {
	Player::Player(glm::vec3 position) {
		transform.position = position;
		transform.rotation = glm::vec3(0.0f);
		transform.scale = glm::vec3(1.0f);
		xRotationDirection = RX_NONE;
		yRotationDirection = RY_NONE;
	}
}

namespace game {
	void updatePlayer(gobjs::Player &player, float dt)
	{
		State* state = State::get();	

		if(state->getKeyState(GLFW_KEY_D) == JUST_PRESSED)	
			player.yRotationDirection = gobjs::Player::RY_RIGHT;	
		else if(state->getKeyState(GLFW_KEY_A) == JUST_PRESSED)	
			player.yRotationDirection = gobjs::Player::RY_LEFT;
		else if(state->getKeyState(GLFW_KEY_A) == RELEASED && 
				state->getKeyState(GLFW_KEY_D) == RELEASED)
			player.yRotationDirection = gobjs::Player::RY_NONE;			

		if(player.yRotationDirection == gobjs::Player::RY_RIGHT) {
			player.transform.rotation.z += dt * ROTATION_Z_SPEED;
			player.transform.rotation.z =
				std::min(player.transform.rotation.z, MAX_ROTATION_Z);
			player.transform.rotation.y -= ROTATION_Y_SPEED * dt;
		}
		else if(player.yRotationDirection == gobjs::Player::RY_LEFT) {
			player.transform.rotation.z -= dt * ROTATION_Z_SPEED;
			player.transform.rotation.z =
				std::max(player.transform.rotation.z, -MAX_ROTATION_Z);
			player.transform.rotation.y += ROTATION_Y_SPEED * dt;
		}
		else {
			if(player.transform.rotation.z < 0.0f)
				player.transform.rotation.z += dt * ROTATION_Z_SPEED / 2.0f;
			else if(player.transform.rotation.z > 0.0f)
				player.transform.rotation.z -= dt * ROTATION_Z_SPEED / 2.0f;
		
			if(std::abs(player.transform.rotation.z) < glm::radians(0.5f))
				player.transform.rotation.z = 0.0f;
		}
		
		if(state->getKeyState(GLFW_KEY_W) == JUST_PRESSED)
			player.xRotationDirection = gobjs::Player::RX_UP;
		else if(state->getKeyState(GLFW_KEY_S) == JUST_PRESSED)
			player.xRotationDirection = gobjs::Player::RX_DOWN;
		else if(state->getKeyState(GLFW_KEY_S) == RELEASED &&
				state->getKeyState(GLFW_KEY_W) == RELEASED)
			player.xRotationDirection = gobjs::Player::RX_NONE;

		if(player.xRotationDirection == gobjs::Player::RX_UP) {
			player.transform.rotation.x -= dt * ROTATION_X_SPEED;
			player.transform.rotation.x =
				std::max(player.transform.rotation.x, -MAX_ROTATION_X);
		}
		else if(player.xRotationDirection == gobjs::Player::RX_DOWN) {
			player.transform.rotation.x += dt * ROTATION_X_SPEED;
			player.transform.rotation.x =
				std::min(player.transform.rotation.x, MAX_ROTATION_X);
		}

		player.transform.position += player.transform.direction() * SPEED * dt;
	}

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
