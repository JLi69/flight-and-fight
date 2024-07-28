#include "infworld.hpp"
#include <random>
#include <glad/glad.h>
#include <chrono>
#include <thread>

namespace infworld {
	worldseed makePermutations(int seed, unsigned int count)
	{
		worldseed permutations(count);
		std::minstd_rand lcg(seed);

		for(int i = 0; i < count; i++)
			rng::createPermutation(permutations[i], lcg());

		return permutations;
	}

	float smoothstep(float x)
	{
		return x * x * (3.0f - 2.0f * x);
	}

	float interpolate(float x, float lowerx, float upperx, float a, float b)
	{
		return (x - lowerx) / (upperx - lowerx) * (b - a) + a;
	}

	float getHeight(float x, float z, const worldseed &permutations) 
	{
		float height = 0.0f;
		float freq = FREQUENCY;
		float amplitude = 1.0f;

		for(int i = 0; i < permutations.size(); i++) {
			float h = perlin::noise(x / freq, z / freq, permutations[i]) * amplitude;
			height += h;
			freq /= 2.0f;
			amplitude /= 2.0f;
		}

		if(height < -0.1f)
			height = interpolate(height, -1.0f, -0.1f, -1.0f, 0.003f);
		else if(height >= -0.1f && height < 0.0f)
			height = interpolate(height, -0.1f, 0.0f, 0.003f, 0.03f);
		else if(height >= 0.0f && height < 0.15f)
			height = interpolate(height, 0.0f, 0.15f, 0.03f, 0.12f);
		else if(height >= 0.1f)
			height = interpolate(height, 0.15f, 1.0f, 0.12f, 1.0f);

		return height; //normalized to be between -1.0 and 1.0
	}

	glm::vec3 getTerrainVertex(
		float x,
		float z,
		const worldseed &permutations,
		float maxheight
	) {
		float h = getHeight(x, z, permutations) * maxheight;
		if(h <= 0.0f)
			h = std::min(-0.007f, h);
		else if(h >= 0.0f)
			h = std::max(0.007f, h);
		return glm::vec3(x, h, z);
	}

	mesh::ElementArrayBuffer<float> createChunkElementArray(
		const worldseed &permutations,
		int chunkx,
		int chunkz,
		float maxheight,
		float chunkscale
	) {
		mesh::ElementArrayBuffer<float> worldarraybuffer;

		worldarraybuffer.mesh.vertices.reserve(PREC * PREC * 3 * 2);

		for(unsigned int i = 0; i <= PREC; i++) {
			for(unsigned int j = 0; j <= PREC; j++) {
				float x = -chunkscale + float(i) / float(PREC) * chunkscale * 2.0f;
				float z = -chunkscale + float(j) / float(PREC) * chunkscale * 2.0f;
				float tx = x + float(chunkx) * chunkscale * 2.0f;
				float tz = z + float(chunkz) * chunkscale * 2.0f;

				glm::vec3 vertex = getTerrainVertex(tx, tz, permutations, maxheight);

				glm::vec3
					v1 = getTerrainVertex(tx + 0.01f, tz, permutations, maxheight),
					v2 = getTerrainVertex(tx, tz + 0.01f, permutations, maxheight),
					norm = glm::normalize(glm::cross(v2 - vertex, v1 - vertex));
				glm::vec2 n = gfx::compressNormal(norm);

				worldarraybuffer.mesh.vertices.push_back(vertex.y / maxheight);	
				worldarraybuffer.mesh.vertices.push_back(n.x);
				worldarraybuffer.mesh.vertices.push_back(n.y);
			}
		}

		return worldarraybuffer;
	}

	ChunkData buildChunk(
		const infworld::worldseed &permutations,
		int x,
		int z,
		float maxheight,
		float chunkscale
	) {
		return {
			infworld::createChunkElementArray(permutations, x, z, maxheight, chunkscale),
			{ x, z }
		};
	}

	inline unsigned int joinBuilderThreads(
		std::vector<std::thread> &builders,
		const std::vector<ChunkData> &builtchunks,
		ChunkTable &chunks,
		unsigned int ind,
		unsigned int maxsz
	) {
		if(builders.size() < maxsz)
			return 0;
		for(auto &th : builders)
			th.join();
		for(int i = 0; i < builders.size(); i++)
			chunks.addChunk(ind + i, builtchunks.at(i));
		unsigned int sz = builders.size();
		builders.clear();
		return sz;
	}

	ChunkTable buildWorld(
		unsigned int range,
		const infworld::worldseed &permutations,
		float maxheight,
		float chunkscale 
	) {
		auto starttime = std::chrono::steady_clock::now();
		unsigned int threadcount = 
			std::max<unsigned int>(std::thread::hardware_concurrency(), 4);
	
		std::vector<ChunkData> builtchunks(threadcount);
		ChunkTable chunks(range, chunkscale, maxheight);
		chunks.genBuffers();
		auto build = 
			[&builtchunks, &permutations, &maxheight, &chunkscale]
			(int x, int z, int i) {
				builtchunks[i] = buildChunk(permutations, x, z, maxheight, chunkscale);
			};
		
		//Multithreading to speed up terrain generation/building
		std::vector<std::thread> builders; 
		int ind = 0;
		for(int x = -int(range); x <= int(range); x++) {
			for(int z = -int(range); z <= int(range); z++) {
				builders.push_back(std::thread(build, x, z, builders.size()));
				unsigned int sz = 	
					joinBuilderThreads(builders, builtchunks, chunks, ind, threadcount);
				ind += sz;
			}
		}

		joinBuilderThreads(builders, builtchunks, chunks, ind, 0);

		auto endtime = std::chrono::steady_clock::now();
		std::chrono::duration<double> duration = endtime - starttime;
		double time = duration.count();
		printf("Time to generate world: %f\n", time);
	
		return chunks;
	}

	std::vector<unsigned int> generateChunkIndices()
	{
		std::vector<unsigned int> indices;

		for(unsigned int i = 0; i < PREC; i++) {
			for(unsigned int j = 0; j < PREC; j++) {
				unsigned int index = i * (PREC + 1) + j;
				indices.push_back(index + (PREC + 1));
				indices.push_back(index + 1);
				indices.push_back(index);

				indices.push_back(index + 1);
				indices.push_back(index + (PREC + 1));
				indices.push_back(index + (PREC + 1) + 1);
			}
		}

		return indices;
	}
}
