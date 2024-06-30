#define _USE_MATH_DEFINES
#include <math.h>
#include "plants.hpp"
#include "gfx.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <stack>

struct BranchProperties {
	glm::mat4 transform = glm::mat4(1.0f); //rotation and scale
	glm::vec3 position = glm::vec3(0.0f);
	unsigned int depth = 0; //How far up the branch is
};

BranchProperties getTop(const std::stack<BranchProperties> &stack)
{
	BranchProperties properties;
	if(!stack.empty())
		properties = stack.top();
	return properties;
}

mesh::Model createBranchSegment(
	const BranchProperties &branch,
	float thickness,
	float decreaseAmt,
	float length,
	unsigned int detail
) {	
	float radius1 = std::max(thickness - decreaseAmt * float(branch.depth), 0.01f);
	float radius2 = std::max(thickness - decreaseAmt * float(branch.depth + 1), 0.01f);
	mesh::Model segment = mesh::createFrustumModel(detail, radius1, radius2);
	
	glm::mat4 transformtc(1.0f);
	transformtc = glm::translate(transformtc, glm::vec3(0.01f, 0.0f, 0.0f));
	transformtc = glm::scale(transformtc, glm::vec3(0.48f, 2.0f, 1.0f));
	mesh::transformModelTc(segment, transformtc);
	
	//Transform the model
	glm::mat4 translate = glm::translate(glm::mat4(1.0f), branch.position);
	//By default the frustum has height 1.0 so we scale it up
	glm::mat4 scaleHeight = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, length, 1.0f));
	glm::mat4 transform = translate * branch.transform * scaleHeight;
	mesh::transformModel(segment, transform);

	return segment;
}

mesh::Model createBranchEnd(
	const BranchProperties &branch,
	float thickness,
	float decreaseAmt,
	float length,
	unsigned int detail
) {
	mesh::Model endsegment = mesh::createConeModel1(detail);
	
	glm::mat4 transformtc(1.0f);
	transformtc = glm::translate(transformtc, glm::vec3(0.01f, 0.0f, 0.0f));
	transformtc = glm::scale(transformtc, glm::vec3(0.48f, 2.0f, 1.0f));
	mesh::transformModelTc(endsegment, transformtc);

	float radius = std::max(thickness - decreaseAmt * float(branch.depth), 0.01f);
	mesh::transformModel(
		endsegment, 
		glm::scale(
			glm::mat4(1.0f),
			glm::vec3(radius, length, radius)
		)
	);
	
	//Transform the model
	glm::mat4 translate = glm::translate(glm::mat4(1.0f), branch.position);
	glm::mat4 transform = translate * branch.transform;
	mesh::transformModel(endsegment, transform);

	//Leaves
	float scale = 1.0f;
	if(branch.depth <= 2)
		scale = 0.5f;

	transformtc = glm::translate(glm::mat4(1.0f), glm::vec3(0.51f, 0.0f, 0.0f));
	transformtc = glm::scale(transformtc, glm::vec3(0.48f, 1.0f, 1.0f));
	int leafcount = std::max<int>((2 * detail) / 3 - 1, 1);
	int leafdetail = std::max<int>(detail / 2 - 2, 0);
	for(int i = 0; i < leafcount; i++) {
		mesh::Model leaves = mesh::createPlaneModel(leafdetail);
		mesh::transformModelTc(leaves, transformtc);

		glm::vec3 offset = 
			(1.0f - scale) * 
			glm::vec3(branch.transform * glm::vec4(0.0f, length, 0.0f, 1.0f));

		glm::mat4 translate = 
			glm::translate(
				glm::mat4(1.0f),
				branch.position + offset
			);
		glm::mat4 rotation = 
			glm::rotate(
				glm::mat4(1.0f),
				glm::radians(120.0f) * i, glm::vec3(0.0f, 1.0f, 0.0f)
			);
		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
		glm::mat4 transform =
			translate *
			branch.transform *
			rotation *
			scaleMat *
			glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		mesh::transformModel(leaves, transform);
		endsegment = mesh::mergeModels(endsegment, leaves);
	}

	return endsegment;
}

namespace plants {
	std::string lsystem(
		unsigned int iterations,
		const std::string &axiom,
		const std::string &rule
	) {
		std::string result = axiom;

		for(int i = 0; i < iterations; i++) {
			std::string currentIteration = result;
			result.clear();
			for(int j = 0; j < currentIteration.size(); j++) {
				if(currentIteration.at(j) == 'F')
					result.append(rule);
				else
					result += currentIteration.at(j);
			}
		}

		return result;
	}	

	mesh::Model createPlantFromStr(
		const std::string &str,
		float angle, 
		float length,
		float thickness,
		float decreaseAmt,
		unsigned int detail
	) {
		const std::unordered_map<char, glm::mat4> rotations = {
			{ '<', glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 0.0f, 0.0f)) },
			{ '>', glm::rotate(glm::mat4(1.0f), -angle, glm::vec3(1.0f, 0.0f, 0.0f)) },
			{ '&', glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f)) },
			{ '+', glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f)) },
			{ '-', glm::rotate(glm::mat4(1.0f), -angle, glm::vec3(0.0f, 0.0f, 1.0f)) },
		};

		mesh::Model plant;
	
		//Create leaves and branches
		std::stack<BranchProperties> branchStack;

		BranchProperties branch;
		for(int i = 0; i < str.size(); i++) {
			char ch = str.at(i);
			mesh::Model treepart;

			if(rotations.count(ch)) {
				branch.transform *= rotations.at(ch);
				continue;
			}

			switch(ch) {
			case 'F':
				if(i == str.size() - 1 || str.at(i + 1) == ']')
					treepart = createBranchEnd(branch, thickness, decreaseAmt, length, detail);
				else
					treepart = createBranchSegment(branch, thickness, decreaseAmt, length, detail);
				branch.position += glm::vec3(branch.transform * glm::vec4(0.0f, length, 0.0f, 1.0f));
				branch.depth++;
				plant = mesh::mergeModels(plant, treepart);
				break;
			case '[':
				branchStack.push(branch);
				break;
			case ']':
				branch = getTop(branchStack);				
				branchStack.pop();
				break;
			default:
				break;
			}
		}

		return plant;
	}

	gfx::Vao createPineTreeModel(unsigned int detail)
	{
		gfx::Vao pinetree;
		//Index 4 is the instance offset array
		pinetree.genBuffers(5);
		pinetree.bind();

		glm::mat4 transform;
		glm::mat4 transformtc;

		//Generate trunk
		mesh::Model treemodel = mesh::createConeModel1(detail);
		transform = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 5.0f, 0.3f));
		mesh::transformModel(treemodel, transform);
		transformtc = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 8.0f, 1.0f));
		mesh::transformModelTc(treemodel, transformtc);

		if(detail > 4) {
			mesh::Model bottom = mesh::createFrustumModel(detail, 0.3f, 0.3f);
			mesh::transformModel(
				bottom, 
				glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
			);
			transformtc = glm::mat4(1.0f);
			transformtc = glm::translate(transformtc, glm::vec3(0.01f, 0.0f, 0.0f));
			transformtc = glm::scale(transformtc, glm::vec3(0.48f, 8.0f / 5.0f, 1.0f));
			mesh::transformModelTc(bottom, transformtc);
			treemodel = mesh::mergeModels(treemodel, bottom);
		}

		//Generate rest of the pine tree
		glm::vec3 top = glm::vec3(0.0f, 5.0f, 0.0f);
		transformtc = glm::mat4(1.0f);
		transformtc = glm::translate(transformtc, glm::vec3(0.51f, 0.02f, 0.0f));
		transformtc = glm::scale(transformtc, glm::vec3(0.48f, 0.96f, 1.0f));
		for(int i = 0; i < 4; i++) {
			mesh::Model part = mesh::createConeModel2(detail);
			float scale = 0.75f + 0.25f * std::pow(1.5f, float(i));
			float height = 1.6f + 0.2f * float(i);
			transform = glm::mat4(1.0f);
			transform = glm::translate(transform, top - glm::vec3(0.0f, height, 0.0f));
			transform = glm::scale(transform, glm::vec3(scale, height, scale));
			float rotation = float(M_PI) / 16.0f * float(i);
			transform = glm::rotate(transform, rotation, glm::vec3(0.0f, 1.0f, 0.0f));
			mesh::transformModel(part, transform);	
			mesh::transformModelTc(part, transformtc);
			
			treemodel = mesh::mergeModels(treemodel, part);
			top.y -= scale * 0.6f;
		}	

		pinetree.vertcount = treemodel.indices.size();	
		treemodel.dataToBuffers(pinetree.buffers);
		glBindBuffer(GL_ARRAY_BUFFER, pinetree.buffers.at(4));
		glVertexAttribPointer(3, 3, GL_FLOAT, false, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(3);
		glVertexAttribDivisor(3, 1);

		return pinetree;
	}

	gfx::Vao createTreeModel(unsigned int detail)
	{
		gfx::Vao tree;
		//Index 4 is the instance offset array
		tree.genBuffers(5);
		tree.bind();

		const float ANGLE = glm::radians(30.0f);
		const float LENGTH = 0.8f;
		const float THICKNESS = 0.15f;
		const float DECREASE = 0.025f;
		const unsigned int ITERATIONS = 2;
		const std::string RULE = "F[&&>F]F[--F][&&&&-F][&&&&&&&&-F]";

		std::string str = lsystem(ITERATIONS, "F", RULE);
		mesh::Model treemodel = createPlantFromStr(
			str,
			ANGLE,
			LENGTH,
			THICKNESS,
			DECREASE,
			detail
		);

		if(detail > 4) {
			mesh::Model bottom = mesh::createFrustumModel(detail, THICKNESS, THICKNESS);
			mesh::transformModel(
				bottom,
				glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
			);
			glm::mat4 transformtc = glm::mat4(1.0f);
			transformtc = glm::translate(transformtc, glm::vec3(0.01f, 0.0f, 0.0f));
			transformtc = glm::scale(transformtc, glm::vec3(0.48f, 8.0f / 5.0f, 1.0f));
			mesh::transformModelTc(bottom, transformtc);
			treemodel = mesh::mergeModels(treemodel, bottom);
		}

		tree.vertcount = treemodel.indices.size();
		treemodel.dataToBuffers(tree.buffers);
		glBindBuffer(GL_ARRAY_BUFFER, tree.buffers.at(4));
		glVertexAttribPointer(3, 3, GL_FLOAT, false, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(3);
		glVertexAttribDivisor(3, 1);

		return tree;
	}
}
