#include "shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

std::string readShaderFile(const char* path)
{
	std::ifstream shaderFile(path);

	if(!shaderFile) {
		std::cerr << "Failed to open " << path << '\n';
		return "";
	}

	std::string line;
	std::stringstream shaderFileContents;
	//Read file line by line and concatonate them together
	while(std::getline(shaderFile, line))
		shaderFileContents << line << '\n';
	shaderFile.close();
	return shaderFileContents.str();
}

unsigned int createShader(const char* path, GLenum shaderType)
{
	unsigned int shader = glCreateShader(shaderType);

	//Read shader file
	std::string shaderCode = readShaderFile(path);
	const char *shaderCodeBegin = shaderCode.c_str();
	const int len = shaderCode.size();
	glShaderSource(shader, 1, &shaderCodeBegin, &len);

	//Compile the shader
	glCompileShader(shader);

	int compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	//Shader failed to compile
	if(compileStatus != 1) {
		std::cerr << path << " failed to compile!\n";

		//Output any compiler errors
		char message[1024]; //For now, we will just assume all error
							//messages < 1024 character long
		int len;
		glGetShaderInfoLog(shader, 1023, &len, message);	
		message[len] = '\0';
		std::cerr << message << '\n';
	}

	return shader;
}

ShaderProgram::ShaderProgram(unsigned int vertex, unsigned int fragment)
{
	programid = glCreateProgram();
	
	glAttachShader(programid, vertex);
	glAttachShader(programid, fragment);
	glLinkProgram(programid);
	glValidateProgram(programid);
	
	//Check for linker errors
	int linkStatus;
	glGetProgramiv(programid, GL_LINK_STATUS, &linkStatus);
	//Failed to link
	if(linkStatus != 1) {
		//Output linker errors
		std::cerr << "Failed to link program!\n";

		char message[1024];
		int len;
		glGetProgramInfoLog(programid, 1023, &len, message);
		std::cerr << message << '\n';
	}

	glDetachShader(programid, vertex);
	glDetachShader(programid, fragment);
}

ShaderProgram::ShaderProgram(const char *vertpath, const char *fragpath)
{
	programid = glCreateProgram();

	unsigned int vertex = createShader(vertpath, GL_VERTEX_SHADER),
				 fragment = createShader(fragpath, GL_FRAGMENT_SHADER);
	glAttachShader(programid, vertex);
	glAttachShader(programid, fragment);
	glLinkProgram(programid);
	glValidateProgram(programid);
	
	//Check for linker errors
	int linkStatus;
	glGetProgramiv(programid, GL_LINK_STATUS, &linkStatus);
	//Failed to link
	if(linkStatus != 1) {
		//Output linker errors
		std::cerr << "Failed to link program!\n";

		char message[1024];
		int len;
		glGetProgramInfoLog(programid, 1023, &len, message);
		std::cerr << message << '\n';
	}

	glDetachShader(programid, vertex);
	glDetachShader(programid, fragment);
	//Clean up
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void ShaderProgram::use()
{
	glUseProgram(programid);
}

int ShaderProgram::getUniformLocation(const char *uniformName)
{
	if(uniformLocations.count(uniformName) == 0) {
		int location = glGetUniformLocation(programid, uniformName);
		uniformLocations[uniformName] = location;
		return location;
	}

	return uniformLocations[uniformName];
}

int ShaderProgram::getUniformBlockIndex(const char *uniformBlockName)
{
	int index = glGetUniformBlockIndex(programid, uniformBlockName);
	return index;
}

void ShaderProgram::setBinding(const char *uniformBlockName, unsigned int binding)
{
	int index = getUniformBlockIndex(uniformBlockName);
	glUniformBlockBinding(programid, index, binding);
}

void ShaderProgram::uniformMat3x3(const char *uniformName, const glm::mat3 &mat)
{
	int location = getUniformLocation(uniformName);
	glUniformMatrix3fv(location, 1, false, glm::value_ptr(mat));
}

void ShaderProgram::uniformMat4x4(const char *uniformName, const glm::mat4 &mat)
{
	int location = getUniformLocation(uniformName);
	glUniformMatrix4fv(location, 1, false, glm::value_ptr(mat));
}

void ShaderProgram::uniformVec4(const char *uniformName, const glm::vec4 &vec)
{
	int location = getUniformLocation(uniformName);
	glUniform4f(location, vec.x, vec.y, vec.z, vec.w);
}

void ShaderProgram::uniformVec3(const char *uniformName, const glm::vec3 &vec)
{
	int location = getUniformLocation(uniformName);
	glUniform3f(location, vec.x, vec.y, vec.z);
}

void ShaderProgram::uniformVec2(const char *uniformName, const glm::vec2 &vec)
{
	int location = getUniformLocation(uniformName);
	glUniform2f(location, vec.x, vec.y);
}

void ShaderProgram::uniformFloat(const char *uniformName, float value)
{
	int location = getUniformLocation(uniformName);
	glUniform1f(location, value);
}

void ShaderProgram::uniformInt(const char *uniformName, int value)
{
	int location = getUniformLocation(uniformName);
	glUniform1i(location, value);
}

unsigned int ShaderProgram::getid()
{
	return programid;
}
