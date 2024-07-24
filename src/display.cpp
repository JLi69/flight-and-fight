#include "display.hpp"
#include "assets.hpp"
#include "app.hpp"
#include "infworld.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

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
		glEnable(GL_CULL_FACE);
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

		for(int i = 0; i < maxlod; i++) {
			terrainShader.uniformFloat("chunksz", chunktables[i].scale());

			if(i < maxlod - 1) {
				float chunkscale = 
					chunktables[i + 1].scale() * 
					2.0f * 
					float(PREC) / float(PREC + 1);
				float range = float(chunktables[i + 1].range()) / lodscale - 0.5f;
				//Slight amount of overlap to mitigate cracks in terrain
				//We increase this amount due to the terrain becoming less precise
				//and more likely to have cracks
				float d = 8.0f * float(i) + 4.0f;
				float maxrange = chunkscale * range * SCALE + d;
				infworld::ChunkPos center = chunktables[i + 1].getCenter();
				
				glm::vec2 centerpos = glm::vec2(float(center.z), float(center.x));
				centerpos *= float(PREC) / float(PREC + 1);
				centerpos *= chunktables[i + 1].scale() * SCALE * 2.0f;
				
				terrainShader.uniformFloat("maxrange", maxrange);
				terrainShader.uniformVec2("center", centerpos);
			}
			else {
				terrainShader.uniformVec2("center", glm::vec2(0.0f));
				terrainShader.uniformFloat("maxrange", -1.0f);
			}

			if(i == 0)
				drawCount += chunktables[i].draw(terrainShader, viewfrustum);
			else {
				int minrange = chunktables[i - 1].range() / int(lodscale);
				drawCount += chunktables[i].draw(terrainShader, minrange, viewfrustum);
			}
		}

		return drawCount;
	}	
}
