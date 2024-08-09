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
}
