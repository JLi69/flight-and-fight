/*
 * This file contains functions for generating plants to make the terrain
 * look pretty and nice
 * */

#pragma once
#include "gfx.hpp"
#include <string>
#include <unordered_map>

namespace plants {
	//Function based off of this article: https://ilchoi.weebly.com/procedural-tree-generator.html
	//This function will use the same characters used the article
	//(F = grow forward, + = rotate along z axis, - = minus rotation along z axis, etc)
	//This is a somewhat limited version of L systems as it only replaces F with
	//the string 'rule' and everything else remains constant but it should
	//work well enough for tree generation
	//
	//iterations = number of iterations to do
	//axiom = starting string
	//rule = string to replace 'F' with
	std::string lsystem(
		unsigned int iterations,
		const std::string &axiom,
		const std::string &rule
	);
	//More detail on what the characters mean:
	// + = positive rotation along z axis
	// - = negative rotation along z axis
	// & = positive rotation along y axis
	// < = positive rotation along x axis
	// > = negative rotation along x axis
	// [ = push matrix
	// ] = pop matrix
	// F = grow
	//
	//angle is the amount each branch is rotated upon a rotation operation
	//length is the length of each branch segment
	//thickness is the starting thickness of the tree at the bottom,
	//it decreases as we go up the tree
	//decreaseAmt the factor by which the branches decrease in thickness as we
	//go up the tree
	//
	//For texture coordinates, we assume that the left half of the texture is
	//the bark/stem/etc and the right half is the leaves texture
	mesh::Model createPlantFromStr(
		const std::string &str,
		float angle,
		float length,
		float thickness,
		float decreaseAmt,
		unsigned int detail
	);
	gfx::Vao createPineTreeModel(unsigned int detail);	
	gfx::Vao createTreeModel(unsigned int detail);
}
