#pragma once

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <map>

typedef unsigned int ShaderId;

//Helper function that reads contents of a shader file, 
//returns the string containing the content,
//takes path of shader as argument
std::string readShaderFile(const char *path);
//Helper function that compiles a shader
//Takes a path of the shader file,
//also takes an argument for the type of shader
//reads the contents, then compiles the shader
//and returns the id of the shader
//will output any compiler errors to stderr
unsigned int createShader(const char *path, GLenum shaderType);

class ShaderProgram {
	std::map<std::string, int> uniformLocations;
	unsigned int programid;
public:
	//creates a shader program by taking in two shader ids,
	//one that is a vertex shader and the other that is
	//the fragment shader, will output any compiler
	//or linker errors to stderr
	ShaderProgram(unsigned int vertex, unsigned int fragment);
	//creates a shader by taking the path of a vertex and fragment shader
	ShaderProgram(const char *vertpath, const char *fragpath);
	void use();
	int getUniformLocation(const char *uniformName);
	int getUniformBlockIndex(const char *uniformBlockName);
	void setBinding(const char *uniformBlockName, unsigned int binding);
	unsigned int getid();

	void uniformMat3x3(const char *uniformName, const glm::mat3 &mat);
	void uniformMat4x4(const char *uniformName, const glm::mat4 &mat);
	void uniformVec4(const char *uniformName, const glm::vec4 &vec);
	void uniformVec3(const char *uniformName, const glm::vec3 &vec);
	void uniformVec2(const char *uniformName, const glm::vec2 &vec);
	void uniformFloat(const char *uniformName, float value);
	void uniformInt(const char *uniformName, int value);
};
