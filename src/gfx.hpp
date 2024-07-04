#pragma once
#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>
#include <string>

namespace mesh {
	template<typename T>
	struct Mesh {
		std::vector<T> vertices;
		size_t size() const
		{
			return vertices.size() * sizeof(T);
		}
		
		const void* ptr() const
		{
			return &vertices[0];
		}
	};

	//Mesh of floats
	typedef Mesh<float> Meshf;

	template<typename T>
	struct ElementArrayBuffer {
		Mesh<T> mesh;
		std::vector<unsigned int> indices;
	};
	
	void addToMesh(Meshf &mesh, const glm::vec3 &v);
	void addToMesh(Meshf &mesh, const glm::vec2 &v);	

	//Combination of vertex, normal, and texture coordinate data
	struct Model {
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texturecoords;
		std::vector<unsigned int> indices;

		Meshf vertData() const;
		Meshf normalData() const;
		Meshf tcData() const;
		void dataToBuffers(const std::vector<unsigned int> &buffers) const;
	};
	
	//Creates a cone model with normal, vertex data, and texture coordinates
	//These cone models do not have bottoms
	//This model has texture coordinates that "wraps" a rectangular texture
	//around the cone
	Model createConeModel1(unsigned int prec);
	//This model has texture coordinates that has texture coordinates that takes
	//a circular section of a texture, the center of the circle is the tip of the
	//cone and the edges of the circle are the bottom edge of the cone
	Model createConeModel2(unsigned int prec);
	//Makes a frustum model (does not have a top or bottom)
	//radius1 is the bottom of the frustum, radius2 is the top of the frustum
	Model createFrustumModel(unsigned int prec, float radius1, float radius2);
	//Makes a plane model
	Model createPlaneModel(unsigned int subdivision);
	//Transforms a model using a transformation matrix
	void transformModel(Model &model, const glm::mat4 &transform);
	//Transforms texture coordinates of a model
	void transformModelTc(Model &model, const glm::mat4 &transform);
	//Combines two models, returns the combined model
	Model mergeModels(const Model &model1, const Model &model2);
	//Loads a model from an obj file, uses the fast_obj library as a dependency
	Model loadObjModel(const char *path);
}

namespace gfx {
	struct Vao {
		unsigned int vertcount;
		unsigned int vaoid;
		std::vector<unsigned int> buffers;
		void genBuffers(unsigned int count);
		void bind() const;
	};
	//Create a quad vao (only position vectors - no texture/normal data)
	Vao createQuadVao();
	//Create a cube vao (only position vectors - not texture/normal data)
	Vao createCubeVao();
	//Creates a vao from a model (has normal and texture coordinate data)
	Vao createModelVao(const mesh::Model &model);
	void destroyVao(Vao &vao);

	//Outputs opengl errors
	void outputErrors();
	//Converts channels to image format
	//(1 = RED, 3 = RGB, 4 = RGBA)
	GLenum getFormat(int channels); 
	//Loads a texture from 'path' and passes its data to textureid
	//returns true if texture is successfully read, false otherwise
	//also has a flag for whether the image should be flipped vertically
	bool loadTexture(const char *path, unsigned int textureid, bool flipvertical);
	//Attempts to load all 6 faces of a cubemap from a vector of 6 paths
	//If loading a face fails, the function will return false, otherwise true
	bool loadCubemap(const std::vector<std::string> &faces, unsigned int textureid);

	//Converts a normal vector (x, y, z) to a 2d vector consisting of
	//angles that represent the vector (we assume the original 3d vector
	//has magnitude of 1). This can allow for a smaller amount of data to
	//be used in the mesh and improve performance
	glm::vec2 compressNormal(glm::vec3 n);
}
