#include "infworld.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace infworld {
	int getChunkSeed(int x, int z, const worldseed &permutations)
	{
		//Taken from wikipedia
		//do some bit magic to hopefully ensure these values are not too periodic
		const unsigned w = 8 * sizeof(unsigned);
		const unsigned s = w / 2;
		unsigned a = x, b = z;
		a *= 3284157443; 
		b ^= a << s | a >> (w-s);
		b *= 1911520717; 
		a ^= b << s | b >> (w-s);
		a *= 2048419325;

		const rng::permutation256& p = permutations.at(0);
		return p[unsigned(p[unsigned(p[a % 256] + b) % 256] % 256)];
	}

	DecorationTable::DecorationTable(unsigned int sz, float scale)
	{
		size = 2 * sz + 1;
		chunkscale = scale;
		for(int x = -int(sz); x <= int(sz); x++)
			for(int z = -int(sz); z <= int(sz); z++)
				positions.push_back({ x, z });	
		decorations = std::vector<std::vector<Decoration>>(count());
	}

	unsigned int DecorationTable::count()
	{
		return size * size;
	}

	//Draw chunk decorations
	void DecorationTable::drawDecorations(const gfx::Vao &vao) {
		if(!vaoCount.count(vao.vaoid))
			return;
		glDrawElementsInstanced(GL_TRIANGLES, vao.vertcount, GL_UNSIGNED_INT, 0, vaoCount.at(vao.vaoid));
	}

	void DecorationTable::genDecorations(
		const worldseed &permutations,
		DecorationType type,
		unsigned int n,
		int x,
		int z,
		unsigned int index,
		std::minstd_rand0 &lcg
	) {
		float chunksz = chunkscale * 2.0f * float(PREC) / float(PREC + 1);	
		float posx = float(z) * chunksz;
		float posz = float(x) * chunksz;
		unsigned int amount = (unsigned int)(lcg()) % n;

		for(int i = 0; i < amount; i++) {
			float x = float((unsigned int)(lcg()) % PREC) / float(PREC) - 0.5f;
			float z = float((unsigned int)(lcg()) % PREC) / float(PREC) - 0.5f;
			x *= chunksz;
			z *= chunksz;	
			x += posx;
			z += posz;
			float y = getHeight(z, x, permutations);	
			y *= HEIGHT;
			x *= float(PREC) / float(PREC + 1);
			z *= float(PREC) / float(PREC + 1);
			decorations.at(index).push_back({
				glm::vec3(x, y - 0.5f, z),
				type,
			});
		}
	}

	void DecorationTable::generate(const worldseed &permutations, unsigned int index)
	{
		ChunkPos pos = positions.at(index);
		int seed = getChunkSeed(pos.x, pos.z, permutations);
		std::minstd_rand0 lcg;
		lcg.seed(seed);
		genDecorations(permutations, PINE_TREE, 120, pos.x, pos.z, index, lcg);
		genDecorations(permutations, TREE, 36, pos.x, pos.z, index, lcg);

		decorations.at(index).erase(std::remove_if(
			decorations.at(index).begin(),
			decorations.at(index).end(),
			[&permutations](Decoration d) {
				float x = d.position.x / 128.0f;
				float z = d.position.z / 128.0f;
				return perlin::noise(x, z, permutations.at(0)) < 0.0f;
			}
		), decorations.at(index).end());

		decorations.at(index).erase(std::remove_if(
			decorations.at(index).begin(),
			decorations.at(index).end(),
			[](Decoration d) {
				float y = d.position.y / HEIGHT;
				return d.type == TREE && (y < 0.02f || y > 0.2f);
			}
		), decorations.at(index).end());

		decorations.at(index).erase(std::remove_if(
			decorations.at(index).begin(),
			decorations.at(index).end(),
			[](Decoration d) {
				float y = d.position.y / HEIGHT;
				return d.type == PINE_TREE && (y < 0.04f || y > 0.3f);
			}
		), decorations.at(index).end());
	}

	//Generate decorations
	void DecorationTable::genDecorations(const worldseed &permutations)
	{
		for(int i = 0; i < decorations.size(); i++)
			generate(permutations, i);
	}

	bool DecorationTable::genNewDecorations(
		float camerax,
		float cameraz,
		const worldseed &permutations
	) {
		float chunksz = 
			chunkscale * 
			float(PREC) / float(PREC + 1) *
			float(PREC) / float(PREC + 1);
		int
			ix = int(floorf((cameraz + chunksz * SCALE) / (chunksz * SCALE * 2.0f))),
			iz = int(floorf((camerax + chunksz * SCALE) / (chunksz * SCALE * 2.0f)));
		if(ix == centerx && iz == centerz)
			return false;

		int range = (size - 1) / 2;
		std::vector<ChunkPos> newChunks;
		for(int x = ix - range; x <= ix + range; x++) {
			for(int z = iz - range; z <= iz + range; z++) {
				if(labs(x - centerx) <= range && labs(z - centerz) <= range)
					continue;
				newChunks.push_back({ x, z });
			}
		}

		std::vector<unsigned int> indices;
		for(int i = 0; i < positions.size(); i++) {
			ChunkPos pos = positions.at(i);
			if(labs(pos.x - ix) <= range && labs(pos.z - iz) <= range)
				continue;
			indices.push_back(i);
		}

		for(int i = 0; i < indices.size(); i++) {
			unsigned int index = indices.at(i);
			ChunkPos pos = newChunks.at(i);
			positions.at(index) = pos;
			decorations.at(index).clear();
			generate(permutations, index);
		}

		centerx = ix;
		centerz = iz;

		return true;
	}

	void DecorationTable::generateOffsets(
		DecorationType type,
		const gfx::Vao &vao,
		unsigned int minrange,
		unsigned int maxrange
	) {
		if(decorations.size() == 0)
			return;

		std::vector<float> offsets;

		for(int i = 0; i < count(); i++) {
			ChunkPos pos = positions.at(i);

			if((labs(pos.x - centerx) < minrange &&
			    labs(pos.z - centerz) < minrange) ||
			   (labs(pos.x - centerx) >= maxrange ||
				labs(pos.z - centerz) >= maxrange))
				continue;

			for(const auto &decoration : decorations.at(i)) {
				if(decoration.type != type)
					continue;
				offsets.push_back(decoration.position.x * SCALE);
				offsets.push_back(decoration.position.y * SCALE);
				offsets.push_back(decoration.position.z * SCALE);
			}
		}

		if(vaoCount.count(vao.vaoid))
			vaoCount.at(vao.vaoid) = offsets.size() / 3;
		else
			vaoCount.insert({ vao.vaoid, offsets.size() / 3 });

		glBindBuffer(GL_ARRAY_BUFFER, vao.buffers.at(4));
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * offsets.size(), &offsets[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}
