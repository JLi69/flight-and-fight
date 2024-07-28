#pragma once
#include <stdint.h>
#include <vector>
#include <glm/glm.hpp>
#include <random>
#include <unordered_map>
#include "noise.hpp"
#include "gfx.hpp"
#include "geometry.hpp"
#include "shader.hpp"

constexpr unsigned int PREC = 40;
constexpr float CHUNK_SZ = 64.0f;
constexpr float HEIGHT = 270.0f;
constexpr float SCALE = 2.5f;
constexpr float FREQUENCY = 720.0f;
constexpr size_t CHUNK_VERT_SZ = 3;
constexpr size_t CHUNK_VERT_SZ_BYTES = CHUNK_VERT_SZ * sizeof(float);
constexpr unsigned int CHUNK_VERT_COUNT = PREC * PREC * 6;
//2 buffers per chunk:
//0 -> position
//1 -> normals
//2 -> indices
constexpr unsigned int BUFFER_PER_CHUNK = 3;

namespace infworld {
	//We will use a seed value (an integer) to generate multiple
	//pseudorandom permutations to feed into the perlin noise generator
	//for world generation
	typedef std::vector<rng::permutation256> worldseed;

	struct ChunkPos {
		int x = 0, z = 0;
	};

	struct ChunkData {
		mesh::ElementArrayBuffer<float> chunkmesh;
		ChunkPos position;
	};

	enum DecorationType {
		TREE,
		PINE_TREE,
	};

	struct Decoration {
		glm::vec3 position;
		DecorationType type;
	};

	class DecorationTable {
		unsigned int size;
		int centerx = 0, centerz = 0;
		float chunkscale;
		//"Chunk decorations" - this is supposed to represent features such
		//as trees (in this case we only have two types of trees)
		std::vector<std::vector<Decoration>> decorations;
		std::vector<ChunkPos> positions;
		std::unordered_map<unsigned int, unsigned int> vaoCount;

		void genDecorations(
			const worldseed &permutations,
			DecorationType type,
			unsigned int n,
			int x,
			int z,
			unsigned int index,
			std::minstd_rand0 &lcg
		);	
		void generate(const worldseed &permutations, unsigned int index);
	public:
		DecorationTable(unsigned int sz, float scale);
		//Draw chunk decorations
		void drawDecorations(const gfx::Vao &vao);
		//Generate decorations
		void genDecorations(const worldseed &permutations);
		//Returns true if new decorations needed to be generated
		bool genNewDecorations(
			float camerax,
			float cameraz,
			const worldseed &permutations
		);
		void generateOffsets(
			DecorationType type,
			const gfx::Vao &vao,
			unsigned int minrange,
			unsigned int maxrange
		);
		unsigned int count();
	};

	class ChunkTable {
		unsigned int chunkcount;
		unsigned int size;
		float chunkscale;
		float height;
		std::vector<unsigned int> vaoids;
		std::vector<unsigned int> bufferids; 
		std::vector<ChunkPos> chunkpos;
		int centerx = 0, centerz = 0;

		//For generating new chunks
		std::vector<unsigned int> indices;
		std::vector<ChunkPos> newChunks;
	public:
		ChunkTable(unsigned int range, float scale, float h);
		ChunkTable();
		void genBuffers();
		void clearBuffers();
		void addChunk(
			unsigned int index,
			const mesh::ElementArrayBuffer<float> &chunkmesh,
			int x,
			int z
		);
		void addChunk(unsigned int index, const ChunkData &chunk);
		void updateChunk(unsigned int index, const ChunkData &chunk);
		void bindVao(unsigned int index);
		ChunkPos getPos(unsigned int index);
		unsigned int count() const;
		ChunkPos getCenter();
		void setCenter(int x, int z);
		void generateNewChunks(
			float camerax,
			float cameraz,
			const worldseed &permutations
		);
		//returns the number of chunks drawn
		unsigned int draw(ShaderProgram &shader, const geo::Frustum &viewfrustum);
		unsigned int draw(
			ShaderProgram &shader,
			unsigned int minrange,
			const geo::Frustum &viewfrustum
		);
		float scale() const;
		unsigned int range() const;	
	};

	worldseed makePermutations(int seed, unsigned int count);
	float getHeight(float x, float z, const worldseed &permutations);
	float interpolate(float x, float lowerx, float upperx, float a, float b);
	glm::vec3 getTerrainVertex(
		float x,
		float z,
		const worldseed &permutations,
		float maxheight
	);
	mesh::ElementArrayBuffer<float> createChunkElementArray(
		const worldseed &permutations,
		int chunkx,
		int chunkz,
		float maxheight,
		float chunkscale
	);
	ChunkData buildChunk(
		const infworld::worldseed &permutations,
		int x,
		int z,
		float maxheight,
		float chunkscale
	);
	ChunkTable buildWorld(
		unsigned int range,
		const infworld::worldseed &permutations,
		float maxheight,
		float chunkscale
	);
	std::vector<unsigned int> generateChunkIndices();
}

const std::vector<unsigned int> CHUNK_INDICES = infworld::generateChunkIndices();
