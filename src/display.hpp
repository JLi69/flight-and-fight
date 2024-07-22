#pragma once

#include "infworld.hpp"

namespace gfx {
	void displaySkybox();
	void displayWater(float totalTime);
	void displayDecorations(infworld::DecorationTable &decorations, float totalTime);
	unsigned int displayTerrain(infworld::ChunkTable *chunktables, int maxlod, float lodscale);
	//lightdir must be a normalized vector (vector of length 1.0)
	void displayModel(
		const std::string &shadername,
		const std::string &texturename,
		const std::string &vaoname,
		const glm::mat4 &transform,
		const glm::vec3 &lightdir
	);
}
