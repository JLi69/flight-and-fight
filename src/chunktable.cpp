#include "infworld.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace infworld {
	//Default constructor
	ChunkTable::ChunkTable()
	{
		size = 0;
		chunkcount = 0;
		chunkscale = 0.0f;
	}

	ChunkTable::ChunkTable(unsigned int range, float scale, float h)
	{
		size = 2 * range + 1;
		chunkcount = size * size;
		chunkscale = scale;
		height = h;
		vaoids = std::vector<unsigned int>(chunkcount);
		chunkpos = std::vector<infworld::ChunkPos>(chunkcount);
		bufferids = std::vector<unsigned int>(BUFFER_PER_CHUNK * chunkcount);
	}

	void ChunkTable::genBuffers()
	{	
		glGenVertexArrays(vaoids.size(), &vaoids[0]);	
		glGenBuffers(bufferids.size(), &bufferids[0]);
	}

	void ChunkTable::clearBuffers()
	{
		glDeleteVertexArrays(vaoids.size(), &vaoids[0]);
		glDeleteBuffers(bufferids.size(), &bufferids[0]);
	}

	void ChunkTable::addChunk(
		unsigned int index,
		const mesh::ElementArrayBuffer<float> &chunkmesh,
		int x,
		int z
	) {
		chunkpos.at(index) = { x, z };

		glBindVertexArray(vaoids.at(index));

		//Buffer 0 (vertex positions)
		glBindBuffer(GL_ARRAY_BUFFER, bufferids.at(index * BUFFER_PER_CHUNK));
		glBufferData(
			GL_ARRAY_BUFFER, 
			chunkmesh.mesh.vertices.size() * sizeof(float),
			&chunkmesh.mesh.vertices[0],
			GL_STATIC_DRAW
		);
		glVertexAttribPointer(
			0,
			1,
			GL_FLOAT,
			false,
			CHUNK_VERT_SZ_BYTES,
			(void*)0
		);
		glEnableVertexAttribArray(0);

		//Buffer 1 (vertex normals)
		glBindBuffer(GL_ARRAY_BUFFER, bufferids.at(index * BUFFER_PER_CHUNK + 1));
		glBufferData(
			GL_ARRAY_BUFFER,
			chunkmesh.mesh.vertices.size() * sizeof(float),
			&chunkmesh.mesh.vertices[0],
			GL_STATIC_DRAW
		);
		glVertexAttribPointer(
			1,
			2,
			GL_FLOAT, 
			false,
			CHUNK_VERT_SZ_BYTES,
			(void*)(sizeof(float))
		);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferids.at(index * BUFFER_PER_CHUNK + 2));
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			CHUNK_INDICES.size() * sizeof(unsigned int),
			&CHUNK_INDICES[0],
			GL_STATIC_DRAW
		);
	}

	void ChunkTable::addChunk(unsigned int index, const ChunkData &chunk)
	{
		addChunk(index, chunk.chunkmesh, chunk.position.x, chunk.position.z);
	}

	void ChunkTable::updateChunk(unsigned int index, const ChunkData &chunk)
	{
		chunkpos.at(index) = { chunk.position.x, chunk.position.z };

		glBindVertexArray(vaoids.at(index));

		//Buffer 0 (vertex positions)
		glBindBuffer(GL_ARRAY_BUFFER, bufferids.at(index * BUFFER_PER_CHUNK));
		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			chunk.chunkmesh.mesh.vertices.size() * sizeof(float),
			&chunk.chunkmesh.mesh.vertices[0]
		);

		//Buffer 1 (vertex normals)
		glBindBuffer(GL_ARRAY_BUFFER, bufferids.at(index * BUFFER_PER_CHUNK + 1));
		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			chunk.chunkmesh.mesh.vertices.size() * sizeof(float),
			&chunk.chunkmesh.mesh.vertices[0]
		);
	}

	void ChunkTable::bindVao(unsigned int index)
	{
		glBindVertexArray(vaoids.at(index));
	}	

	infworld::ChunkPos ChunkTable::getPos(unsigned int index)
	{
		return chunkpos.at(index);
	}

	unsigned int ChunkTable::count() const
	{
		return chunkcount;
	}

	ChunkPos ChunkTable::getCenter() 
	{
		return { centerx, centerz };
	}

	void ChunkTable::setCenter(int x, int z)
	{
		centerx = x;
		centerz = z;
	}

	void ChunkTable::generateNewChunks(
		float camerax,
		float cameraz,
		const worldseed &permutations
	) {
		if(indices.size() > 0) {
			int i = indices.size() - 1;
			int x = newChunks.at(i).x, z = newChunks.at(i).z;
			ChunkData chunk = buildChunk(permutations, x, z, height, chunkscale);
			updateChunk(indices.at(i), chunk);
			indices.pop_back();
			newChunks.pop_back();
			return;
		}

		float chunksz = chunkscale * float(PREC) / float(PREC + 1);
		int
			ix = int(floorf((cameraz + chunksz * SCALE) / (chunksz * SCALE * 2.0f))),
			iz = int(floorf((camerax + chunksz * SCALE) / (chunksz * SCALE * 2.0f)));
		if(ix == centerx && iz == centerz)
			return;

		int range = (size - 1) / 2;
		for(int x = ix - range; x <= ix + range; x++) {
			for(int z = iz - range; z <= iz + range; z++) {
				if(labs(x - centerx) <= range && labs(z - centerz) <= range)
					continue;
				newChunks.push_back({ x, z });
			}
		}

		//Determine which chunks are out of range
		for(int i = 0; i < chunkcount; i++) {
			int 
				chunkx = chunkpos.at(i).x,
				chunkz = chunkpos.at(i).z;	
			if(labs(ix - chunkx) <= range && labs(iz - chunkz) <= range)
				continue;
			indices.push_back(i);
		}	

		centerx = ix;
		centerz = iz;
	}

	unsigned int ChunkTable::draw(
		ShaderProgram &shader,
		const geo::Frustum &viewfrustum
	) {
		unsigned int drawCount = 0;
		for(int i = 0; i < count(); i++) {
			infworld::ChunkPos p = getPos(i);

			float x = float(p.z) * chunkscale * 2.0f * float(PREC) / float(PREC + 1);
			float z = float(p.x) * chunkscale * 2.0f * float(PREC) / float(PREC + 1);	

			geo::AABB chunkAABB = geo::AABB(
				glm::vec3(x, 0.0f, z) * SCALE,
				glm::vec3(chunkscale * 2.0f, HEIGHT * 2.0f, chunkscale * 2.0f) * SCALE
			);

			if(!geo::intersectsFrustum(viewfrustum, chunkAABB))
				continue;

			glm::mat4 transform = glm::mat4(1.0f);
			transform = glm::scale(transform, glm::vec3(SCALE));
			transform = glm::translate(transform, glm::vec3(x, 0.0f, z));
			shader.uniformMat4x4("transform", transform);
			bindVao(i);
			glDrawElements(GL_TRIANGLES, CHUNK_VERT_COUNT, GL_UNSIGNED_INT, 0);
			drawCount++;
		}

		return drawCount;
	}

	unsigned int ChunkTable::draw(
		ShaderProgram &shader,
		unsigned int minrange,
		const geo::Frustum &viewfrustum
	) {
		unsigned int drawCount = 0;
		for(int i = 0; i < count(); i++) {
			infworld::ChunkPos p = getPos(i);

			if(std::abs(p.x - centerx) < minrange && 
				std::abs(p.z - centerz) < minrange)
				continue;

			float x = float(p.z) * chunkscale * 2.0f * float(PREC) / float(PREC + 1);
			float z = float(p.x) * chunkscale * 2.0f * float(PREC) / float(PREC + 1);	
		
			geo::AABB chunkAABB = geo::AABB(
				glm::vec3(x, 0.0f, z) * SCALE,
				glm::vec3(chunkscale * 2.0f, HEIGHT * 2.0f, chunkscale * 2.0f) * SCALE
			);
			
			//Frustum culling
			if(!geo::intersectsFrustum(viewfrustum, chunkAABB))
				continue;

			glm::mat4 transform = glm::mat4(1.0f);
			transform = glm::scale(transform, glm::vec3(SCALE));
			transform = glm::translate(transform, glm::vec3(x, 0.0f, z));
			shader.uniformMat4x4("transform", transform);
			bindVao(i);
			glDrawElements(GL_TRIANGLES, CHUNK_VERT_COUNT, GL_UNSIGNED_INT, 0);
			drawCount++;
		}

		return drawCount;
	}

	float ChunkTable::scale() const
	{
		return chunkscale;
	}

	unsigned int ChunkTable::range() const
	{
		return (size - 1) / 2;
	}
}
