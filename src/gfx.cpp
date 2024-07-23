#define _USE_MATH_DEFINES
#include <math.h>
#include "gfx.hpp"
#include <stdio.h>
#include <stb_image/stb_image.h>
#include <fast_obj/fast_obj.h>
#include <assert.h>
#include <unordered_map>
#include <sstream>

namespace mesh {
	void addToMesh(Meshf &mesh, const glm::vec3 &v)
	{
		mesh.vertices.push_back(v.x);
		mesh.vertices.push_back(v.y);
		mesh.vertices.push_back(v.z);
	}

	void addToMesh(Meshf &mesh, const glm::vec2 &v)
	{
		mesh.vertices.push_back(v.x);
		mesh.vertices.push_back(v.y);
	}

	void Model::dataToBuffers(const std::vector<unsigned int> &buffers) const
	{
		//index 0 = vertices
		//index 1 = texture coordinates
		//index 2 = normals
		//index 3 = indices
		assert(buffers.size() >= 4);	

		mesh::Meshf 
			vertdata = vertData(),
			tcdata = tcData(),
			normData = normalData();
		//vertex positions
		glBindBuffer(GL_ARRAY_BUFFER, buffers.at(0));	
		glBufferData(GL_ARRAY_BUFFER, vertdata.size(), vertdata.ptr(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		//texture coordinates
		glBindBuffer(GL_ARRAY_BUFFER, buffers.at(1));
		glBufferData(GL_ARRAY_BUFFER, tcdata.size(), tcdata.ptr(), GL_STATIC_DRAW);	
		glVertexAttribPointer(1, 2, GL_FLOAT, false, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		//normals
		glBindBuffer(GL_ARRAY_BUFFER, buffers.at(2));	
		glBufferData(GL_ARRAY_BUFFER, normData.size(), normData.ptr(), GL_STATIC_DRAW);	
		glVertexAttribPointer(2, 3, GL_FLOAT, false, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(2);
		//indices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.at(3));
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			indices.size() * sizeof(unsigned int),
			&indices[0],
			GL_STATIC_DRAW
		);
	}

	Model createConeModel1(unsigned int prec)
	{
		Model cone;

		const glm::vec3 top = glm::vec3(0.0f, 1.0f, 0.0f);
		cone.vertices.push_back(top);
		cone.normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
		cone.texturecoords.push_back(glm::vec2(0.5f, 1.0f));

		for(int i = 0; i < prec; i++) {
			//Calculate vertex
			float angle = 2.0f * M_PI / float(prec) * float(i);
			glm::vec3 vert = glm::vec3(cosf(angle), 0.0f, sinf(angle));
			cone.vertices.push_back(vert);

			//Compute texture coordinates
			float tcx;
			float fract = float(i % (prec / 2)) / float(prec - 1);
			if(i < prec / 2)
				tcx = std::min(fract * 2.0f, 1.0f);
			else
				tcx = std::max(1.0f - fract * 2.0f, 0.0f);
			cone.texturecoords.push_back(glm::vec2(tcx, 0.0f));

			//Compute normals
			float nextangle = 2.0f * M_PI / float(prec) * float(i + 1);
			glm::vec3 nextvert = glm::vec3(cosf(nextangle), 0.0f, sinf(nextangle));
			glm::vec3 norm = glm::normalize(glm::cross(top - vert, nextvert - vert));
			cone.normals.push_back(norm);

			//Calculate indices
			unsigned int index1 = i + 1;
			unsigned int index2 = index1 + 1;
			if(index2 >= prec + 1)
				index2 = 1;
			cone.indices.push_back(index2);
			cone.indices.push_back(index1);
			cone.indices.push_back(0);
		}

		return cone;
	}

	Model createConeModel2(unsigned int prec)
	{
		Model cone;

		const glm::vec3 top = glm::vec3(0.0f, 1.0f, 0.0f);
		cone.vertices.push_back(top);
		cone.normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
		cone.texturecoords.push_back(glm::vec2(0.5f, 0.5f));

		for(int i = 0; i < prec; i++) {
			//Calculate vertex
			float angle = 2.0f * M_PI / float(prec) * float(i);
			glm::vec3 vert = glm::vec3(cosf(angle), 0.0f, sinf(angle));
			cone.vertices.push_back(vert);

			//Compute normals
			float nextangle = 2.0f * M_PI / float(prec) * float(i + 1);
			glm::vec3 nextvert = glm::vec3(cosf(nextangle), 0.0f, sinf(nextangle));
			glm::vec3 norm = glm::normalize(glm::cross(top - vert, nextvert - vert));
			cone.normals.push_back(norm);

			//Calculate texture coordinates
			glm::vec2 tc = 
				glm::vec2(cosf(angle), sinf(angle)) * 0.5f + 
				glm::vec2(0.5f, 0.5f);
			cone.texturecoords.push_back(tc);

			//Calculate indices
			unsigned int index1 = i + 1;
			unsigned int index2 = index1 + 1;
			if(index2 >= prec + 1)
				index2 = 1;
			cone.indices.push_back(index2);
			cone.indices.push_back(index1);
			cone.indices.push_back(0);
		}

		return cone;
	}

	Model createFrustumModel(unsigned int prec, float radius1, float radius2)
	{
		Model frustum;

		for(int i = 0; i < prec; i++) {
			float angle = float(i) / float(prec) * 2.0f * M_PI;
			//Calculate vertex
			glm::vec3 vert = glm::vec3(cosf(angle), 0.0f, sinf(angle)) * radius1;
			//Calculate texture coordinates
			float tcx;
			float fract = float(i % (prec / 2)) / float(prec - 1);
			if(i < prec / 2)
				tcx = std::min(fract * 2.0f, 1.0f);
			else
				tcx = std::max(1.0f - fract * 2.0f, 0.0f);
			//Calculate normals
			glm::vec3 top = 
				glm::vec3(cosf(angle), 0.0f, sinf(angle)) * radius2 +
				glm::vec3(0.0f, 1.0f, 0.0f);
			float nextangle = float(i + 1) / float(prec) * 2.0f * M_PI;
			glm::vec3 nextvert = 
				glm::vec3(cosf(nextangle), 0.0f, sinf(nextangle)) * radius1;
			glm::vec3 norm = glm::normalize(glm::cross(top - vert, nextvert - vert));

			frustum.vertices.push_back(vert);
			frustum.texturecoords.push_back(glm::vec2(tcx, 0.0f));
			frustum.normals.push_back(norm);
		}

		for(int i = 0; i < prec; i++) {
			float angle = float(i) / float(prec) * 2.0f * M_PI;
			//Calculate vertex
			glm::vec3 vert = 
				glm::vec3(cosf(angle), 0.0f, sinf(angle)) * radius2 +
				glm::vec3(0.0f, 1.0f, 0.0f);
			//Calculate texture coordinates
			float tcx;
			float fract = float(i % (prec / 2)) / float(prec - 1);
			if(i < prec / 2)
				tcx = std::min(fract * 2.0f, 1.0f);
			else
				tcx = std::max(1.0f - fract * 2.0f, 0.0f);
			//Calculate normals
			glm::vec3 bot = 
				glm::vec3(cosf(angle), 0.0f, sinf(angle)) * radius1;
			float nextangle = float(i + 1) / float(prec) * 2.0f * M_PI;
			glm::vec3 nextvert = 
				glm::vec3(cosf(nextangle), 0.0f, sinf(nextangle)) * radius2 +
				glm::vec3(0.0f, 1.0f, 0.0f);
			glm::vec3 norm = glm::normalize(glm::cross(vert - bot, nextvert - vert));	

			frustum.vertices.push_back(vert);
			frustum.texturecoords.push_back(glm::vec2(tcx, 1.0f));
			frustum.normals.push_back(norm);
		}

		//Calculate indices
		for(int i = 0; i < prec; i++) {
			frustum.indices.push_back(i);
			frustum.indices.push_back((i + 1) % prec);
			frustum.indices.push_back(i + prec);

			frustum.indices.push_back(i + prec);
			frustum.indices.push_back((i + 1) % prec + prec);
			frustum.indices.push_back((i + 1) % prec);
		}

		return frustum;
	}

	Model createPlaneModel(unsigned int subdivision)
	{
		Model plane;	

		for(int i = 0; i <= subdivision + 2; i++) {
			for(int j = 0; j <= subdivision + 2; j++) {
				float x = float(j) / float(subdivision + 2) * 2.0f - 1.0f;
				float y = float(i) / float(subdivision + 2) * 2.0f - 1.0f;
				float tcx = float(j) / float(subdivision + 2);
				float tcy = float(i) / float(subdivision + 2);
				plane.vertices.push_back(glm::vec3(x, y, 0.0f));
				plane.normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
				plane.texturecoords.push_back(glm::vec2(tcx, tcy));
			}
		}

		for(int i = 0; i <= subdivision + 1; i++) {
			for(int j = 0; j <= subdivision + 1; j++) {
				plane.indices.push_back(i * (subdivision + 3) + j);
				plane.indices.push_back(i * (subdivision + 3) + j + 1);
				plane.indices.push_back((i + 1) * (subdivision + 3) + j);

				plane.indices.push_back((i + 1) * (subdivision + 3) + j);
				plane.indices.push_back((i + 1) * (subdivision + 3) + j + 1);
				plane.indices.push_back(i * (subdivision + 3) + j + 1);
			}
		}

		return plane;
	}

	void transformModel(Model &model, const glm::mat4 &transform)
	{
		for(auto &vert : model.vertices)
			vert = transform * glm::vec4(vert.x, vert.y, vert.z, 1.0f);
		const glm::mat4 normalMat = glm::transpose(glm::inverse(transform));
		for(auto &norm : model.normals) {
			norm = normalMat * glm::vec4(norm.x, norm.y, norm.z, 1.0f);
			norm = glm::normalize(norm);
		}
	}

	void transformModelTc(Model &model, const glm::mat4 &transform)
	{
		for(auto &tc : model.texturecoords)
			tc = transform * glm::vec4(tc.x, tc.y, 0.0f, 1.0f);
	}

	Model mergeModels(const Model &model1, const Model &model2)
	{
		Model merged;

		//Copy model data into merged model
		merged.vertices = model1.vertices;
		merged.normals = model1.normals;
		merged.texturecoords = model1.texturecoords;
		merged.indices = model1.indices;

		//Copy model 2's data to the merged model (adjust vertices as needed)
		for(auto &vert : model2.vertices)
			merged.vertices.push_back(vert);
		for(auto &norm : model2.normals)
			merged.normals.push_back(norm);
		for(auto &tc : model2.texturecoords)
			merged.texturecoords.push_back(tc);
		unsigned int startingindex = model1.vertices.size();
		for(auto index : model2.indices)
			merged.indices.push_back(startingindex + index);

		return merged;
	}

	std::string indicesToStr(unsigned int v, unsigned int t, unsigned int n)
	{
		std::stringstream ss;
		ss << v << '/' << t << '/' << n;
		return ss.str();
	}

	Model loadObjModel(const char *path)
	{
		fastObjMesh* m = fast_obj_read(path);

		if(!m) {
			fast_obj_destroy(m);
			fprintf(stderr, "Failed to open file: %s\n", path);
			return mesh::Model();
		}

		mesh::Model model;

		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texturecoords;

		vertices.reserve(m->position_count);
		for(int i = 0; i < m->position_count; i++) {
			int index = i * 3;
			glm::vec3 pos(
				m->positions[index],
				m->positions[index + 1],
				m->positions[index + 2]
			);
			vertices.push_back(pos);
		}

		normals.reserve(m->normal_count);
		for(int i = 0; i < m->normal_count; i++) {	
			int index = i * 3;
			glm::vec3 norm(
				m->normals[index],
				m->normals[index + 1],
				m->normals[index + 2]
			);
			normals.push_back(norm);
		} 

		texturecoords.reserve(m->texture_count);
		for(int i = 0; i < m->texcoord_count; i++) {
			int index = i * 2;
			glm::vec2 tc(m->texcoords[index], m->texcoords[index + 1]);
			texturecoords.push_back(tc);
		}

		std::unordered_map<std::string, unsigned int> indices;
		model.indices.reserve(m->index_count);
		unsigned int index = 0;
		for(int i = 0; i < m->index_count; i++) {
			fastObjIndex ind = m->indices[i];
			std::string indstr = indicesToStr(ind.p, ind.t, ind.n);
			if(indices.count(indstr))
				model.indices.push_back(indices.at(indstr));	
			else {
				model.vertices.push_back(vertices.at(ind.p));
				model.normals.push_back(normals.at(ind.n));
				model.texturecoords.push_back(texturecoords.at(ind.t));
				model.indices.push_back(index);
				indices.insert({ indstr, index });
				index++;
			}
		}

		fast_obj_destroy(m);

		return model;
	}

	Meshf Model::vertData() const 
	{
		Meshf data;
		for(auto vert : vertices)
			addToMesh(data, vert);
		return data;
	}

	Meshf Model::normalData() const 
	{
		Meshf data;
		for(auto norm : normals)
			addToMesh(data, norm);
		return data;
	}

	Meshf Model::tcData() const
	{
		Meshf data;
		for(auto tc : texturecoords)
			addToMesh(data, tc);
		return data;
	}
}

namespace gfx {
	void Vao::bind() const
	{	
		glBindVertexArray(vaoid);
	}

	void Vao::genBuffers(unsigned int count)
	{
		glGenVertexArrays(1, &vaoid);
		buffers = std::vector<unsigned int>(count);
		glGenBuffers(buffers.size(), &buffers[0]);
	}

	Vao createQuadVao()
	{
		const float QUAD[] = {
			1.0f, 0.0f, 1.0f,
			1.0f, 0.0f, -1.0f,
			-1.0f, 0.0f, 1.0f,
			-1.0f, 0.0f, -1.0f,
		};
		
		const unsigned int QUAD_INDICES[] = {
			0, 1, 2,
			1, 3, 2,
		};

		Vao quadvao;
		quadvao.buffers = std::vector<unsigned int>(2);
		glGenVertexArrays(1, &quadvao.vaoid);
		glBindVertexArray(quadvao.vaoid);
		glGenBuffers(2, &quadvao.buffers[0]);
		glBindBuffer(GL_ARRAY_BUFFER, quadvao.buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD), QUAD, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadvao.buffers[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(QUAD_INDICES), QUAD_INDICES, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);

		quadvao.vertcount = 6;
		return quadvao;
	}

	Vao createCubeVao()
	{
		const float CUBE[] = {
			1.0f, 1.0f, 1.0f, //0
			-1.0f, 1.0f, 1.0f, //1
			-1.0f, -1.0f, 1.0f, //2
			1.0f, -1.0f, 1.0f, //3
			1.0f, 1.0f, -1.0f, //4
			-1.0f, 1.0f, -1.0f, //5
			-1.0f, -1.0f, -1.0f, //6
			1.0f, -1.0f, -1.0f, //7
		};

		const unsigned int CUBE_INDICES[] = {
			7, 6, 5,
    		5, 4, 7,

    		5, 6, 2,
    		2, 1, 5,

    		0, 3, 7,
    		7, 4, 0,

    		0, 1, 2,
    		2, 3, 0,

    		0, 4, 5,
			5, 1, 0,

    		7, 2, 6,
    		3, 2, 7,
		};

		Vao cubevao;
		cubevao.buffers = std::vector<unsigned int>(2);
		glGenVertexArrays(1, &cubevao.vaoid);
		glBindVertexArray(cubevao.vaoid);
		glGenBuffers(2, &cubevao.buffers[0]);
		glBindBuffer(GL_ARRAY_BUFFER, cubevao.buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE), CUBE, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubevao.buffers[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CUBE_INDICES), CUBE_INDICES, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		cubevao.vertcount = 36;
		return cubevao;
	}

	Vao createModelVao(const mesh::Model &model)
	{
		Vao vao;
		vao.genBuffers(4);
		vao.bind();
		model.dataToBuffers(vao.buffers);
		vao.vertcount = model.indices.size();
		return vao;
	}

	void destroyVao(Vao &vao) 
	{
		glDeleteVertexArrays(1, &vao.vaoid);
		glDeleteBuffers(vao.buffers.size(), &vao.buffers[0]);
		vao.vertcount = 0;
		vao.buffers.clear();
	}

	void outputErrors()
	{
		GLenum err = glGetError();
		int errorcount = 0;
		while(err != GL_NO_ERROR) {
			fprintf(stderr, "OpenGL error: %d\n", err);
			err = glGetError();
			errorcount++;
		}
	
		if(errorcount > 0)
			fprintf(stderr, "%d error(s)\n", errorcount);
	}

	GLenum getFormat(int channels) 
	{
		switch(channels) {
		case 1:
			return GL_RED;
		case 3:
			return GL_RGB;
		case 4:
			return GL_RGBA;
		}
		return GL_RGBA;
	}

	bool loadTexture(const char *path, unsigned int textureid, bool flipvertical)
	{
		stbi_set_flip_vertically_on_load(flipvertical);
		int width, height, channels;
		unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
		bool success = false;
		if(data) {
			success = true;
			GLenum format = getFormat(channels);
			glBindTexture(GL_TEXTURE_2D, textureid);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				format,
				width,
				height,
				0,
				format,
				GL_UNSIGNED_BYTE, 
				data
			);	
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
			fprintf(stderr, "Failed to open: %s\n", path);
	
		stbi_set_flip_vertically_on_load(false); //reset flag to false
		stbi_image_free(data);
		return success;
	}

	bool loadCubemap(const std::vector<std::string> &faces, unsigned int textureid)
	{
		bool success = true;
		int width, height, channels;
		assert(faces.size() == 6); //faces must have 6 elements in it
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureid);

		for(int i = 0; i < 6; i++) {
			unsigned char* data = 
				stbi_load(faces.at(i).c_str(), &width, &height, &channels, 0);

			if(data) {	
				GLenum format = getFormat(channels);
				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0,
					format,
					width,
					height,
					0,
					format,
					GL_UNSIGNED_BYTE, 
					data
				);
			}
			else {
				fprintf(stderr, "Failed to open cubemap file: %s\n", faces.at(i).c_str()); 
				success = false;
			}

			stbi_image_free(data);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		return success;
	}

	float getAngle(float x, float y)
	{
		if(x == 0.0f && y > 0.0f)
			return M_PI / 2.0f;
		if(x == 0.0f && y < 0.0f)
			return M_PI / 2.0f * 3.0f;

		float angle = atanf(y / x);

		if(x > 0.0f && y < 0.0f)
			angle += M_PI * 2.0f;
		else if(x < 0.0f && y > 0.0f)
			angle += M_PI;
		else if(x < 0.0f && y < 0.0f)
			angle += M_PI;

		return angle;
	}

	glm::vec2 compressNormal(glm::vec3 n)
	{
		return glm::vec2(getAngle(n.x, n.z), asinf(n.y));
	}
}
