#include "game.hpp"
#include "assets.hpp"
#include "app.hpp"
#include "infworld.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

//Just debug colors for different terrain LOD levels
constexpr glm::vec3 TERRAIN_LOD_COLORS[] = {
	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(0.0f, 0.0f, 1.0f),
	glm::vec3(1.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 1.0f),
	glm::vec3(1.0f, 0.0f, 1.0f),
};

constexpr float MINIMAP_SIZE = 80.0f;

namespace gobjs = gameobjects;

namespace gfx {
	void displaySkybox() 
	{
		State* state = State::get();
		Camera& cam = state->getCamera();

		//Draw skybox
		glCullFace(GL_FRONT);
		TEXTURES->bindTexture("skybox", GL_TEXTURE0);	
		ShaderProgram& skyboxShader = SHADERS->getShader("skybox");
		skyboxShader.use();
		
		//Uniforms
		skyboxShader.uniformInt("skybox", 0);
		skyboxShader.uniformMat4x4("persp", state->getPerspective());
		glm::mat4 skyboxView = glm::mat4(glm::mat3(cam.viewMatrix()));	
		skyboxShader.uniformMat4x4("view", skyboxView);

		VAOS->bind("cube");
		VAOS->draw();
		glCullFace(GL_BACK);
	}

	void displayWater(float totalTime)
	{
		State* state = State::get();
		Camera& cam = state->getCamera();
		
		VAOS->bind("quad");
		const int waterrange = 4;
		const int count = (waterrange * 2 + 1) * (waterrange * 2 + 1);
		const float quadscale = CHUNK_SZ * 32.0f * SCALE;
		//Draw water
		ShaderProgram& waterShader = SHADERS->getShader("water");
		waterShader.use();
		TEXTURES->bindTexture("watermaps", GL_TEXTURE0);	
		waterShader.uniformInt("range", waterrange);
		waterShader.uniformFloat("scale", quadscale);
		waterShader.uniformInt("waternormals", 0);
		waterShader.uniformInt("waterdudv", 1);
		waterShader.uniformMat4x4("persp", state->getPerspective());
		waterShader.uniformMat4x4("view", cam.viewMatrix());
		waterShader.uniformVec3("lightdir", glm::normalize(glm::vec3(-1.0f)));
		waterShader.uniformVec3("camerapos", cam.position);
		waterShader.uniformFloat("time", totalTime);
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, glm::vec3(cam.position.x, 0.0f, cam.position.z));
		transform = glm::scale(transform, glm::vec3(quadscale));
		waterShader.uniformMat4x4("transform", transform);
		VAOS->drawInstanced(count);
	}

	void generateDecorationOffsets(infworld::DecorationTable &decorations)
	{
		decorations.generateOffsets(infworld::PINE_TREE, VAOS->getVao("pinetree"), 0, 4);
		decorations.generateOffsets(infworld::PINE_TREE, VAOS->getVao("pinetreelowdetail"), 4, 999);
		decorations.generateOffsets(infworld::TREE, VAOS->getVao("tree"), 0, 4);
		decorations.generateOffsets(infworld::TREE, VAOS->getVao("treelowdetail"), 4, 8);
	}

	void displayDecorations(
		infworld::DecorationTable &decorations,
		float totalTime
	) {
		State* state = State::get();
		Camera& cam = state->getCamera();

		glDisable(GL_CULL_FACE);
		//Display trees	
		ShaderProgram& treeShader = SHADERS->getShader("tree");
		treeShader.use();
		treeShader.uniformMat4x4("persp", state->getPerspective());
		treeShader.uniformMat4x4("view", cam.viewMatrix());
		treeShader.uniformVec3("lightdir", glm::normalize(glm::vec3(-1.0f)));
		treeShader.uniformVec3("camerapos", cam.position);
		treeShader.uniformFloat("time", totalTime);
		treeShader.uniformFloat("windstrength", SCALE * 3.0f);
		treeShader.uniformMat4x4(
			"transform",
			glm::scale(glm::mat4(1.0f), glm::vec3(SCALE * 2.5f))
		);
		//treeShader.uniformFloat("specularfactor", 0.0f);
		treeShader.uniformVec3("camerapos", cam.position);
		//Draw pine trees
		TEXTURES->bindTexture("pinetree", GL_TEXTURE0);
		VAOS->bind("pinetree");
		decorations.drawDecorations(VAOS->getVao("pinetree"));
		VAOS->bind("pinetreelowdetail");
		decorations.drawDecorations(VAOS->getVao("pinetreelowdetail"));
		//Draw trees
		TEXTURES->bindTexture("tree", GL_TEXTURE0);
		VAOS->bind("tree");
		decorations.drawDecorations(VAOS->getVao("tree"));
		VAOS->bind("treelowdetail");
		decorations.drawDecorations(VAOS->getVao("treelowdetail"));
		glEnable(GL_CULL_FACE);
	}

	unsigned int displayTerrain(infworld::ChunkTable *chunktables, int maxlod, float lodscale)
	{
		State* state = State::get();
		Camera& cam = state->getCamera();
		ShaderProgram& terrainShader = SHADERS->getShader("terrain");

		//Draw terrain
		terrainShader.use();
		//Textures
		TEXTURES->bindTexture("terrain", GL_TEXTURE0);
		terrainShader.uniformInt("terraintexture", 0);
		//uniforms
		terrainShader.uniformMat4x4("persp", state->getPerspective());
		terrainShader.uniformMat4x4("view", cam.viewMatrix());
		terrainShader.uniformVec3("lightdir", glm::normalize(glm::vec3(-1.0f)));
		terrainShader.uniformVec3("camerapos", cam.position);
		unsigned int drawCount = 0;	

		geo::Frustum viewfrustum = cam.getViewFrustum(
			state->getZnear(),
			state->getZfar(),
			state->getAspect(),
			state->getFovy()
		);

		infworld::ChunkPos center = chunktables[0].getCenter();
		glm::vec2 centerpos = glm::vec2(float(center.z), float(center.x));
		centerpos *= float(PREC) / float(PREC + 1);
		centerpos *= chunktables[0].scale() * SCALE * 2.0f;
		terrainShader.uniformVec2("center", centerpos);

		float mindist = 0.0f;
		for(int i = 0; i < maxlod; i++) {
			terrainShader.uniformVec3("testcolor", TERRAIN_LOD_COLORS[i]);
			terrainShader.uniformFloat("chunksz", chunktables[i].scale());

			if(i < maxlod - 1) {
				float chunkscale = 
					chunktables[i].scale() * 
					2.0f * 
					float(PREC) / float(PREC + 1);
				float range = float(chunktables[i].range()) - 0.5f;
				//Slight amount of overlap to mitigate cracks in terrain
				//We increase this amount due to the terrain becoming less precise
				//and more likely to have cracks
				float d = 8.0f * float(i) + 4.0f;
				float maxrange = chunkscale * range * SCALE + d;	
			
				terrainShader.uniformFloat("minrange", mindist);
				terrainShader.uniformFloat("maxrange", maxrange);

				mindist = maxrange - 2.0f * d;
			}
			else {
				terrainShader.uniformFloat("minrange", mindist);
				terrainShader.uniformFloat("maxrange", -1.0f);
			}

			if(i == 0)
				drawCount += chunktables[i].draw(terrainShader, viewfrustum);
			else {
				int minrange = chunktables[i - 1].range() / int(lodscale);
				drawCount += chunktables[i].draw(terrainShader, minrange - 1, viewfrustum);
			}
		}

		return drawCount;
	}

	void displayPlayerPlane(float totalTime, const game::Transform &transform)
	{
		State* state = State::get();
		Camera& cam = state->getCamera();

		ShaderProgram& shader = SHADERS->getShader("textured");	

		shader.use();
		shader.uniformMat4x4("persp", state->getPerspective());
		shader.uniformMat4x4("view", cam.viewMatrix());
		shader.uniformVec3("lightdir", LIGHT);
		shader.uniformVec3("camerapos", cam.position);
	
		//Display plane body
		glm::mat4 transformMat = transform.getTransformMat();
		glm::mat4 normal = glm::mat3(glm::transpose(glm::inverse(transformMat)));
		TEXTURES->bindTexture("plane", GL_TEXTURE0);	
		shader.uniformFloat("specularfactor", 0.5f);	
		shader.uniformMat4x4("transform", transformMat);
		shader.uniformMat3x3("normalmat", normal);
		VAOS->bind("plane");
		VAOS->draw();

		//Display propeller
		TEXTURES->bindTexture("propeller", GL_TEXTURE0);
		glm::mat4 propellerTransform = glm::mat4(1.0f);
		propellerTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 13.888f));
		float rotation = totalTime * 16.0f;
		propellerTransform = glm::rotate(propellerTransform, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
		propellerTransform = transformMat * propellerTransform;
		normal = glm::mat3(glm::transpose(glm::inverse(propellerTransform)));
		shader.uniformFloat("specularfactor", 0.0f);
		shader.uniformMat4x4("transform", propellerTransform);
		shader.uniformMat3x3("normalmat", normal);
		VAOS->bind("propeller");
		VAOS->draw();
	}

	void displayExplosions(const std::vector<gobjs::Explosion> &explosions)
	{
		if(explosions.empty())
			return;

		State* state = State::get();

		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);
		VAOS->bind("quad");
		SHADERS->use("explosion");
		TEXTURES->bindTexture("explosion_particle", GL_TEXTURE0);
		ShaderProgram& shader = SHADERS->getShader("explosion");
		shader.uniformMat4x4("persp", state->getPerspective());
		shader.uniformMat4x4("view", state->getCamera().viewMatrix());
		for(const auto &explosion : explosions) {
			if(!explosion.visible)
				continue;
			shader.uniformFloat("time", explosion.timePassed);
			shader.uniformFloat("scale", explosion.explosionScale);
			shader.uniformMat4x4("transform", explosion.transform.getTransformMat());
			VAOS->drawInstanced(128);
		}
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
	}

	void displayBalloons(const std::vector<gameobjects::Enemy> &balloons)
	{
		if(balloons.empty())
			return;

		State* state = State::get();

		glDisable(GL_CULL_FACE);
		VAOS->bind("balloon");
		SHADERS->use("textured");
		TEXTURES->bindTexture("balloon", GL_TEXTURE0);
		ShaderProgram& shader = SHADERS->getShader("textured");
		shader.uniformMat4x4("persp", state->getPerspective());
		shader.uniformMat4x4("view", state->getCamera().viewMatrix());
		shader.uniformFloat("specularfactor", 0.0f);
		shader.uniformVec3("lightdir", LIGHT);
		shader.uniformVec3("camerapos", state->getCamera().position);
		for(const auto &balloon : balloons) {
			glm::mat4 transform = balloon.transform.getTransformMat();
			glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(transform)));
			shader.uniformMat4x4("transform", transform);
			shader.uniformMat3x3("normalmat", normal);
			VAOS->draw();
		}
		glEnable(GL_CULL_FACE);
	}

	void displayBlimps(const std::vector<gameobjects::Enemy> &blimps)
	{
		if(blimps.empty())
			return;
	
		State* state = State::get();

		VAOS->bind("blimp");
		SHADERS->use("textured");
		TEXTURES->bindTexture("blimp", GL_TEXTURE0);
		ShaderProgram& shader = SHADERS->getShader("textured");
		shader.uniformMat4x4("persp", state->getPerspective());
		shader.uniformMat4x4("view", state->getCamera().viewMatrix());
		shader.uniformFloat("specularfactor", 0.1f);
		shader.uniformVec3("lightdir", LIGHT);
		shader.uniformVec3("camerapos", state->getCamera().position);
		for(const auto &blimp : blimps) {
			glm::mat4 transform = blimp.transform.getTransformMat();
			glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(transform)));
			shader.uniformMat4x4("transform", transform);
			shader.uniformMat3x3("normalmat", normal);
			VAOS->draw();
		}
	}

	void displayUfos(const std::vector<gameobjects::Enemy> &ufos)
	{
		if(ufos.empty())
			return;

		State* state = State::get();
	
		VAOS->bind("ufo");
		SHADERS->use("textured");
		TEXTURES->bindTexture("ufo", GL_TEXTURE0);
		ShaderProgram& shader = SHADERS->getShader("textured");
		shader.uniformMat4x4("persp", state->getPerspective());
		shader.uniformMat4x4("view", state->getCamera().viewMatrix());
		shader.uniformFloat("specularfactor", 1.0f);
		shader.uniformVec3("lightdir", LIGHT);
		shader.uniformVec3("camerapos", state->getCamera().position);
		for(const auto &ufo : ufos) {
			glm::mat4 transform = ufo.transform.getTransformMat();
			glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(transform)));
			shader.uniformMat4x4("transform", transform);
			shader.uniformMat3x3("normalmat", normal);
			VAOS->draw();
		}
	}

	void displayPlanes(float totalTime, const std::vector<gameobjects::Enemy> &planes)
	{
		if(planes.empty())
			return;

		State* state = State::get();

		VAOS->bind("plane");
		SHADERS->use("textured");
		TEXTURES->bindTexture("enemy_plane", GL_TEXTURE0);
		ShaderProgram& shader = SHADERS->getShader("textured");
		shader.uniformMat4x4("persp", state->getPerspective());
		shader.uniformMat4x4("view", state->getCamera().viewMatrix());
		shader.uniformFloat("specularfactor", 0.5f);
		shader.uniformVec3("lightdir", LIGHT);
		shader.uniformVec3("camerapos", state->getCamera().position);
		for(const auto &plane : planes) {
			glm::mat4 transform = plane.transform.getTransformMat();
			glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(transform)));
			shader.uniformMat4x4("transform", transform);
			shader.uniformMat3x3("normalmat", normal);
			VAOS->draw();
		}

		shader.uniformFloat("specularfactor", 0.0f);
		VAOS->bind("propeller");
		TEXTURES->bindTexture("propeller", GL_TEXTURE0);
		for(const auto &plane : planes) {
			glm::mat4 propellerTransform = glm::mat4(1.0f);
			propellerTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 13.888f));
			float rotation = totalTime * 16.0f;
			propellerTransform = glm::rotate(propellerTransform, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
			propellerTransform = plane.transform.getTransformMat() * propellerTransform;
			glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(propellerTransform)));
			shader.uniformMat4x4("transform", propellerTransform);
			shader.uniformMat3x3("normalmat", normal);
			VAOS->draw();
		}
	}

	void displayBullets(const std::vector<gameobjects::Bullet> &bullets)
	{
		if(bullets.empty())
			return;

		State* state = State::get();

		VAOS->bind("bullet");
		TEXTURES->bindTexture("bullet", GL_TEXTURE0);
		SHADERS->use("trail");
		ShaderProgram& trailshader = SHADERS->getShader("trail");
		trailshader.uniformMat4x4("persp", state->getPerspective());
		trailshader.uniformMat4x4("view", state->getCamera().viewMatrix());
		trailshader.uniformFloat("specularfactor", 1.0f);
		trailshader.uniformVec3("lightdir", LIGHT);
		trailshader.uniformVec3("camerapos", state->getCamera().position);
		for(const auto &bullet : bullets) {
			glm::mat4 transform = bullet.transform.getTransformMat();
			glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(transform)));
			glm::vec3 velocity = bullet.transform.direction() * BULLET_SPEED;
			trailshader.uniformFloat("time", bullet.time);
			trailshader.uniformVec3("velocity", velocity);
			trailshader.uniformMat4x4("transform", transform);
			trailshader.uniformMat3x3("normalmat", normal);	
			VAOS->drawInstanced(32);
		}	
	}

	void displayMiniMapBackground()
	{
		State* state = State::get();
		int w, h;
		glfwGetWindowSize(state->getWindow(), &w, &h);	
		glm::mat4 screenMat = 
			glm::scale(glm::mat4(1.0f), glm::vec3(2.0f / float(w), 2.0f / float(h), 0.0f));

		VAOS->bind("quad");
		SHADERS->use("minimap");
	
		//Display minimap background
		ShaderProgram& minimapshader = SHADERS->getShader("minimap");
		minimapshader.uniformMat4x4("screen", screenMat);
		glm::mat4 transform(1.0f);
		transform = glm::translate(transform, glm::vec3(100.0f, 100.0f, 0.0f));
		transform = glm::translate(transform, glm::vec3(-float(w) / 2.0f, -float(h) / 2.0f, 0.0f));
		transform = glm::scale(transform, glm::vec3(MINIMAP_SIZE, MINIMAP_SIZE, 0.0f));
		transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		minimapshader.uniformMat4x4("transform", transform);
		VAOS->draw();

		//Player icon
		TEXTURES->bindTexture("player_marker", GL_TEXTURE0);
		SHADERS->use("textured2d");
		ShaderProgram& texture2dshader = SHADERS->getShader("textured2d");	
		texture2dshader.uniformMat4x4("screen", screenMat);
		transform = glm::mat4(1.0f);
		transform = glm::translate(transform, glm::vec3(100.0f, 100.0f, 0.0f));
		transform = glm::translate(transform, glm::vec3(-float(w) / 2.0f, -float(h) / 2.0f, 0.0f));
		transform = glm::scale(transform, glm::vec3(8.0f, 8.0f, 0.0f));
		transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		texture2dshader.uniformMat4x4("transform", transform);
		VAOS->draw();
	}

	void displayEnemyMarkers(
		const std::vector<gameobjects::Enemy> &enemies,
		const game::Transform &playertransform
	) {
		State* state = State::get();
		int w, h;
		glfwGetWindowSize(state->getWindow(), &w, &h);
		glm::mat4 screenMat = 
			glm::scale(glm::mat4(1.0f), glm::vec3(2.0f / float(w), 2.0f / float(h), 0.0f));

		const float MAX_DIST = CHUNK_SZ * 16.0f;
	
		VAOS->bind("quad");
		SHADERS->use("textured2d");
		TEXTURES->bindTexture("enemy_marker", GL_TEXTURE0);
		ShaderProgram& texture2dshader = SHADERS->getShader("textured2d");
		texture2dshader.uniformMat4x4("screen", screenMat);
		glm::vec2 center(playertransform.position.x, playertransform.position.z);
		for(const auto &enemy : enemies) {
			//Calculate distance to player
			glm::vec2 enemypos(enemy.transform.position.x, enemy.transform.position.z);
			glm::vec2 diff = enemypos - center;
			float dist = glm::length(diff);

			//If we are too far away, don't display the icon
			if(dist > MAX_DIST)
				continue;

			//Caclulate angle with enemy
			glm::vec3 direction = playertransform.direction();
			float dirAngle = compressNormal(glm::normalize(direction)).x;
			diff = glm::normalize(diff);
			float enemyAngle = compressNormal(glm::vec3(diff.x, 0.0f, diff.y)).x;
			float angle = dirAngle - enemyAngle + glm::radians(90.0f);
			dist /= MAX_DIST;

			//Display icon
			glm::mat4 transform = glm::mat4(1.0f);
			float x = MINIMAP_SIZE * cosf(angle) * dist;
			float y = MINIMAP_SIZE * sinf(angle) * dist;
			transform = glm::translate(transform, glm::vec3(x, y, 0.0f));
			transform = glm::translate(transform, glm::vec3(100.0f, 100.0f, 0.0f));
			transform = glm::translate(transform, glm::vec3(-float(w) / 2.0f, -float(h) / 2.0f, 0.0f));
			transform = glm::scale(transform, glm::vec3(8.0f, 8.0f, 0.0f));
			transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			texture2dshader.uniformMat4x4("transform", transform);
			VAOS->draw();
		}
	}

	void displayCrosshair(const game::Transform &playertransform)
	{
		State* state = State::get();
		int w, h;
		glfwGetWindowSize(state->getWindow(), &w, &h);
		glm::mat4 screenMat = 
			glm::scale(glm::mat4(1.0f), glm::vec3(2.0f / float(w), 2.0f / float(h), 0.0f));

		VAOS->bind("quad");
		SHADERS->use("textured2d");
		TEXTURES->bindTexture("crosshair", GL_TEXTURE0);
		ShaderProgram& texture2dshader = SHADERS->getShader("textured2d");
		texture2dshader.uniformMat4x4("screen", screenMat);

		//Calculate the screen position of the crosshair based on where the
		//player is facing
		glm::vec3 worldpos = 
			playertransform.position + 
			playertransform.direction();
		glm::mat4 view = state->getCamera().viewMatrix();
		glm::vec4 screenpos = state->getPerspective() * view * glm::vec4(worldpos, 1.0f);

		//Display
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, glm::vec3(screenpos.x, screenpos.y, 0.0f));
		transform = glm::scale(transform, glm::vec3(8.0f, 8.0f, 0.0f));
		transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		texture2dshader.uniformMat4x4("transform", transform);
		VAOS->draw();
	}
}
