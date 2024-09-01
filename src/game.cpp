#include "game.hpp"
#include "assets.hpp"
#include "plants.hpp"
#include "app.hpp"
#include "audio.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace game {
	Transform::Transform()
	{
		position = glm::vec3(0.0f);
		scale = glm::vec3(1.0f);
		rotation = glm::vec3(0.0f);
	}

	glm::mat4 Transform::getTransformMat() const
	{
		glm::mat4 transform(1.0f);
		transform = glm::translate(transform, position);
		transform = glm::scale(transform, scale);
		transform = glm::rotate(transform, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		transform = glm::rotate(transform, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		transform = glm::rotate(transform, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		return transform;
	}

	glm::vec3 Transform::direction() const
	{
		glm::vec4 dir(0.0f, 0.0f, 1.0f, 1.0f);
		glm::mat4 rotationMat(1.0f);
		rotationMat = glm::rotate(rotationMat, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		rotationMat = glm::rotate(rotationMat, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		rotationMat = glm::rotate(rotationMat, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		dir = rotationMat * dir;
		return glm::normalize(glm::vec3(dir.x, dir.y, dir.z));
	}

	glm::vec3 Transform::right() const 
	{
		return glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), direction()));
	}

	glm::vec3 Transform::rotate(const glm::vec3 &v) const
	{
		glm::mat4 rotationMat(1.0f);
		rotationMat = glm::rotate(rotationMat, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		rotationMat = glm::rotate(rotationMat, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		rotationMat = glm::rotate(rotationMat, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		glm::vec4 transformed(v.x, v.y, v.z, 1.0f);
		transformed = rotationMat * transformed;
		return glm::vec3(transformed.x, transformed.y, transformed.z);
	}

	glm::vec3 Transform::invRotate(const glm::vec3 &v) const
	{
		glm::mat4 rotationMat(1.0f);
		rotationMat = glm::rotate(rotationMat, -rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		rotationMat = glm::rotate(rotationMat, -rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		rotationMat = glm::rotate(rotationMat, -rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		glm::vec4 transformed(v.x, v.y, v.z, 1.0f);
		transformed = rotationMat * transformed;
		return glm::vec3(transformed.x, transformed.y, transformed.z);
	}

	void loadAssets()
	{
		//Vaos
		VAOS->genSimple();
		VAOS->add("pinetree", plants::createPineTreeModel(8));
		VAOS->add("pinetreelowdetail", plants::createPineTreeModel(4));
		VAOS->add("tree", plants::createTreeModel(6));
		VAOS->add("treelowdetail", plants::createTreeModel(3));
		VAOS->importFromFile("assets/models.impfile");
		//Textures
		TEXTURES->importFromFile("assets/textures.impfile");
		//Shaders
		SHADERS->importFromFile("assets/shaders.impfile");	
		//Fonts	
		FONTS->importFromFile("assets/fonts.impfile");
		//Audio
		SFX->importFromFile("assets/sfx.impfile");
	}

	void generateChunks(
		const infworld::worldseed &permutations,
		infworld::ChunkTable *chunktables,
		unsigned int range
	) {
		float sz = CHUNK_SZ;
		for(int i = 0; i < MAX_LOD; i++) {
			chunktables[i] = 
				infworld::buildWorld(range, permutations, HEIGHT, sz);
			sz *= LOD_SCALE;
		}
	}

	void generateNewChunks(
		const infworld::worldseed &permutations,
		infworld::ChunkTable *chunktables,
		infworld::DecorationTable &decorations
	) {
		Camera& cam = State::get()->getCamera();
		for(int i = 0; i < MAX_LOD; i++)
			chunktables[i].generateNewChunks(cam.position.x, cam.position.z, permutations);
		//If we generate new terrain, we must generate new decorations as well
		bool generated = decorations.genNewDecorations(cam.position.x, cam.position.z, permutations);
		if(generated)
			gfx::generateDecorationOffsets(decorations);
	}

	//This initializes the uniform block of values that should be shared across
	//all shaders, it should be at binding 0 and for our purposes any values sent
	//into it should remain constants throughout the execution of the program
	void initGlobalValUniformBlock()
	{
		const float viewdist = 
			CHUNK_SZ * SCALE * 2.0f * float(RANGE) * std::pow(LOD_SCALE, MAX_LOD - 2);
		const float globalShaderVals[] = {
			viewdist,
		};

		unsigned int globalShaderValsUbo;
		glGenBuffers(1, &globalShaderValsUbo);
		glBindBuffer(GL_UNIFORM_BUFFER, globalShaderValsUbo);
		glBufferData(
			GL_UNIFORM_BUFFER,
			sizeof(globalShaderVals),
			globalShaderVals,
			GL_STATIC_DRAW
		);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		SHADERS->getShader("water").setBinding("GlobalVals", 0);
		SHADERS->getShader("tree").setBinding("GlobalVals", 0);
		SHADERS->getShader("terrain").setBinding("GlobalVals", 0);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, globalShaderValsUbo);
	}

	void initUniforms()
	{
		initGlobalValUniformBlock();
		SHADERS->use("terrain");
		SHADERS->getShader("terrain").uniformFloat("maxheight", HEIGHT);
		SHADERS->getShader("terrain").uniformInt("prec", PREC);
	}

	void TimerManager::addTimer(const std::string &name, float maxtime)
	{
		Timer t = {
			.time = maxtime,
			.maxtime = maxtime
		};
		timers.insert({ name, t });
	}

	void TimerManager::addTimer(const std::string& name, float time, float maxtime)
	{
		Timer t = {
			.time = time,
			.maxtime = maxtime
		};
		timers.insert({ name, t });
	}

	void TimerManager::update(float dt)
	{
		for(auto &t : timers)
			t.second.time -= dt;
	}

	void TimerManager::reset()
	{
		for(auto &t : timers)
			if(t.second.time < 0.0f)
				t.second.time = t.second.maxtime;
	}

	bool TimerManager::getTimer(const std::string &name)
	{
		if(!timers.count(name))
			return false;
		return timers.at(name).time < 0.0f;
	}
}
