#include <glm/glm.hpp>
#pragma once

namespace geo {
	struct Plane {
		float d; //Distance to origin
		glm::vec3 norm; //Normal vector defining plane, assume it has length 1
		Plane(float dist, glm::vec3 normal);
		//Create a plane from a single point on it and its normal
		Plane(glm::vec3 pos, glm::vec3 normal);
	};

	struct AABB {
		glm::vec3 pos;
		glm::vec3 dimensions;
		AABB(glm::vec3 position, glm::vec3 size);
	};

	struct Frustum {
		Plane
			back,
			front,
			top,
			bottom,
			left,
			right;
	};

	float signedDist(const Plane &p, const glm::vec3 &pos);
	bool inFront(const Plane &p, const glm::vec3 &pos);
	bool inFront(const Plane &p, const AABB &aabb);
	bool intersectsFrustum(const Frustum &frustum, const AABB &aabb);
};
