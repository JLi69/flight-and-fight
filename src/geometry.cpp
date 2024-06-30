#include "geometry.hpp"

namespace geo {
	Plane::Plane(float dist, glm::vec3 normal)
	{
		d = dist;
		norm = glm::normalize(normal);
	}

	Plane::Plane(glm::vec3 pos, glm::vec3 normal)
	{
		norm = glm::normalize(normal);
		d = glm::dot(pos, norm);
	}

	AABB::AABB(glm::vec3 position, glm::vec3 size)
	{
		pos = position;
		dimensions = size;
	}

	float signedDist(const Plane &p, const glm::vec3 &pos)
	{
		return glm::dot(pos, p.norm) - p.d;
	}

	bool inFront(const Plane &p, const glm::vec3 &pos)
	{
		return signedDist(p, pos) >= 0.0f;
	}

	bool inFront(const Plane &p, const AABB &aabb)
	{
		glm::vec3 extent = aabb.dimensions / 2.0f;
		float r = glm::dot(glm::abs(p.norm), extent);
		return signedDist(p, aabb.pos) >= -r;
	}

	bool intersectsFrustum(const Frustum &frustum, const AABB &aabb)
	{
		return
			inFront(frustum.back, aabb) &&
			inFront(frustum.front, aabb) &&
			inFront(frustum.left, aabb) &&
			inFront(frustum.right, aabb) &&
			inFront(frustum.top, aabb) &&
			inFront(frustum.bottom, aabb);
	}
}
