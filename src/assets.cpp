#include "assets.hpp"
#include "app.hpp"

namespace assets {
	TextureManager* TextureManager::get()
	{
		static TextureManager* texturemanager = new TextureManager;
		return texturemanager;
	}

	void TextureManager::bindTexture(const std::string &name, GLenum texturei) {
		if(!textures.count(name))
			return;
		TextureInfo info = textures.at(name);
		glActiveTexture(texturei);
		glBindTexture(info.target, info.id);
	}

	TextureMetaData entryToTextureMetaData(const impfile::Entry &entry)
	{
		TextureMetaData texture;

		texture.name = entry.name;
		texture.target = entry.getVar("target");

		if(texture.target == "cubemap") {
			texture.cubemapPaths = {
				entry.getVar("east"),
				entry.getVar("west"),
				entry.getVar("up"),
				entry.getVar("down"),
				entry.getVar("north"),
				entry.getVar("south"),
			};
		}
		//Preferably target is 'texture2d' but it defaults to texture2d
		else {
			texture.path = entry.getVar("path");
			std::string flip = entry.getVar("flip");
			if(flip == "true")
				texture.flipv = true;
			//Preferably flip is "false" but if it is an unrecognized value,
			//it will default to false
			else
				texture.flipv = false;
		}	

		return texture;
	}

	TextureInfo textureMetaDataToInfo(
		const TextureMetaData &metadata, 
		unsigned int id
	) {
		TextureInfo info;
		info.id = id;

		if(metadata.target == "cubemap") {
			info.target = GL_TEXTURE_CUBE_MAP;
			gfx::loadCubemap(metadata.cubemapPaths, info.id);
		}
		else {
			info.target = GL_TEXTURE_2D;
			gfx::loadTexture(metadata.path.c_str(), info.id, metadata.flipv);
		}

		return info;
	}

	void TextureManager::importFromFile(const char *path)
	{
		std::vector<impfile::Entry> entries = impfile::parseFile(path);

		std::vector<unsigned int> textureids(entries.size());
		glGenTextures(entries.size(), &textureids[0]);

		for(int i = 0; i < entries.size(); i++) {
			const impfile::Entry& entry = entries.at(i);
			TextureMetaData metadata = entryToTextureMetaData(entry);
			unsigned id = textureids.at(i);
			TextureInfo texinfo = textureMetaDataToInfo(metadata, id);
			textures.insert({ entry.name, texinfo });
		}
	}

	ShaderMetaData entryToShaderMetaData(const impfile::Entry &entry)
	{
		ShaderMetaData metadata;

		metadata.name = entry.name;
		metadata.vertpath = entry.getVar("vertex");
		metadata.fragpath = entry.getVar("fragment");

		return metadata;
	}

	void ShaderManager::importFromFile(const char *path)
	{	
		std::vector<impfile::Entry> entries = impfile::parseFile(path);

		for(int i = 0; i < entries.size(); i++) {
			const impfile::Entry& entry = entries.at(i);
			ShaderMetaData metadata = entryToShaderMetaData(entries.at(i));
			ShaderProgram program(
				metadata.vertpath.c_str(), 
				metadata.fragpath.c_str()
			);
			shaders.insert({ metadata.name, program });
		}
	}

	void ShaderManager::use(const std::string &name)
	{
		if(!shaders.count(name))
			return;
		shaders.at(name).use();
	}

	ShaderProgram& ShaderManager::getShader(const std::string &name)
	{
		if(!shaders.count(name)) {
			fprintf(stderr, "%s does not exist as a shader!\n", name.c_str());
			exit(1); //Crash the program, this hopefully shouldn't happen
		}
		return static_cast<ShaderProgram&>(shaders.at(name));
	}

	ShaderManager* ShaderManager::get()
	{
		static ShaderManager* shadermanager = new ShaderManager;
		return shadermanager;
	}

	ModelMetaData entryToModelMetaData(const impfile::Entry &entry)
	{
		ModelMetaData metadata;
		metadata.name = entry.name;
		metadata.path = entry.getVar("path");
		return metadata;
	}

	VaoManager* VaoManager::get() 
	{
		static VaoManager* vaomanager = new VaoManager;
		return vaomanager;
	}

	void VaoManager::importFromFile(const char *path)
	{
		std::vector<impfile::Entry> entries = impfile::parseFile(path);

		for(const auto &entry : entries) {
			ModelMetaData metadata = entryToModelMetaData(entry);
			add(
				metadata.name,
				gfx::createModelVao(mesh::loadObjModel(metadata.path.c_str()))
			);
		}
	}

	void VaoManager::add(const std::string &name, gfx::Vao vao)
	{
		vaos.insert({ name, vao });
	}

	void VaoManager::genSimple()
	{
		add("quad", gfx::createQuadVao());
		add("cube", gfx::createCubeVao());
	}

	void VaoManager::bind(const std::string &name) 
	{
		if(!vaos.count(name)) {
			fprintf(stderr, "vao %s does not exist!\n", name.c_str()); 
			return;
		}
		vertcount = vaos.at(name).vertcount;
		vaos.at(name).bind();
	}

	void VaoManager::draw()
	{
		glDrawElements(GL_TRIANGLES, vertcount, GL_UNSIGNED_INT, 0);
	}

	void VaoManager::drawInstanced(unsigned int count) 
	{
		glDrawElementsInstanced(GL_TRIANGLES, vertcount, GL_UNSIGNED_INT, 0, count);
	}

	gfx::Vao& VaoManager::getVao(const std::string &name)
	{
		if(!vaos.count(name)) {	
			fprintf(stderr, "vao %s does not exist!\n", name.c_str()); 
			exit(1);
		}
		return vaos.at(name);
	}

	FontMetaData entryToFontMetaData(const impfile::Entry &entry)
	{
		FontMetaData metadata;
		metadata.name = entry.name;
		metadata.path = entry.getVar("path");
		metadata.fontsize = atoi(entry.getVar("fontsz").c_str());
		return metadata;
	}

	void FontManager::importFromFile(const char *path)
	{
		State* state = State::get();
		std::vector<impfile::Entry> entries = impfile::parseFile(path);
	
		//For whatever reason Nuklear crashes if I don't include this code here
		//do not delete this code despite the fact it doesn't load any fonts
		nk_glfw3_font_stash_begin(state->getNkGlfw(), &fontatlas);
		nk_glfw3_font_stash_end(state->getNkGlfw());

		nk_glfw3_font_stash_begin(state->getNkGlfw(), &fontatlas);
		for(const auto &entry : entries) {
			FontMetaData fontmetadata = entryToFontMetaData(entry);
			nk_font* font = nk_font_atlas_add_from_file(
				fontatlas,
				fontmetadata.path.c_str(),
				fontmetadata.fontsize,
				0
			);
			fonts.insert({ fontmetadata.name, font });
		}
		nk_glfw3_font_stash_end(state->getNkGlfw());
	}

	FontManager* FontManager::get()
	{
		static FontManager* fontmanager = new FontManager;
		return fontmanager;
	}

	void FontManager::pushFont(const std::string &fontname)
	{
		State* state = State::get();

		if(!fonts.count(fontname))
			return;

		nk_style_push_font(
			state->getNkContext(),
			&fonts.at(fontname)->handle
		);
	}

	void FontManager::popFont()
	{	
		nk_style_pop_font(State::get()->getNkContext());
	}
}
