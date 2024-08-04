/*
 * This file is for handling imported assets such as textures, models, and
 * shaders and provides some structures for storing such assets in memory
 * */

#pragma once

#include "gfx.hpp"
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_KEYSTATE_BASED_INPUT
#include <nuklear/nuklear.h>
#include <nuklear/nuklear_glfw_gl3.h>
#include "importfile.hpp"
#include "shader.hpp"
#include <unordered_map>
#include <glad/glad.h>

namespace assets {
	struct TextureMetaData {
		std::string name;
		std::vector<std::string> cubemapPaths;
		std::string path;
		std::string target;
		bool flipv;
	};

	struct TextureInfo {
		unsigned int id;
		GLenum target;
	};

	struct ShaderMetaData {
		std::string name;
		std::string vertpath;
		std::string fragpath;
	};

	struct ModelMetaData {
		std::string name;
		std::string path;
	};

	struct FontMetaData {
		std::string name;
		std::string path;
		unsigned int fontsize;
	};

	class TextureManager {
		std::unordered_map<std::string, TextureInfo> textures = {};
		TextureManager() {}
	public:
		static TextureManager* get();
		void importFromFile(const char *path);
		void bindTexture(const std::string &name, GLenum texturei);
	};

	class VaoManager {
		unsigned int vertcount = 0;
		std::unordered_map<std::string, gfx::Vao> vaos = {};
		VaoManager() {}
	public:
		static VaoManager* get();
		//This function will crash the program if you attempt to access
		//a nonexistent vao
		gfx::Vao& getVao(const std::string &name);
		void add(const std::string &name, gfx::Vao vao);
		//Generates simple models such as a quad or cube
		void genSimple();
		void importFromFile(const char *path);
		void bind(const std::string &name);
		void draw();
		void drawInstanced(unsigned int count);
	};

	class ShaderManager {
		std::unordered_map<std::string, ShaderProgram> shaders = {};
		ShaderManager() {}
	public:
		static ShaderManager* get();
		void importFromFile(const char *path);
		void use(const std::string &name);
		//Do not attempt to access a shader that does not exist,
		//it will crash the program
		ShaderProgram& getShader(const std::string &name);
	};

	class FontManager {
		std::unordered_map<std::string, nk_font*> fonts = {};
		nk_font_atlas* fontatlas;
		FontManager() {}
	public:
		void importFromFile(const char *path);
		static FontManager* get();
		void pushFont(const std::string &fontname);
		void popFont();
	};

	//assumes the entry has the following variables:
	//path, target, flip
	//path is the path to the texture (relative to the executable)
	//target is either 'cubemap' or 'texture2d'
	//flip is either 'true' or 'false'
	//however, if it is a cubemap then it include 6 paths:
	//east, west, up, down, north, south
	TextureMetaData entryToTextureMetaData(const impfile::Entry &entry);
	TextureInfo textureMetaDataToInfo(
		const TextureMetaData &metadata, 
		unsigned int id
	);
	//assumes that the entry has the following variables:
	//vertex, fragment
	//'vertex' is the path of the vertex shader relative to the executable
	//'fragment' is the path of the fragment shader relative to the executable
	ShaderMetaData entryToShaderMetaData(const impfile::Entry &entry);
	//Assumes that the entry has the following variables:
	//path
	//'path' is the path to an obj file relative to the executable
	ModelMetaData entryToModelMetaData(const impfile::Entry &entry);
	//Assumes that the entry has the following variables:
	//path, fontsz
	//'path' is the path to a ttf file relative to the executable,
	//fontsz is the font size
	FontMetaData entryToFontMetaData(const impfile::Entry &entry);
}

#define TEXTURES assets::TextureManager::get()
#define SHADERS assets::ShaderManager::get()
#define VAOS assets::VaoManager::get()
#define FONTS assets::FontManager::get()
