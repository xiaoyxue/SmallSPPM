#pragma once

#include <algorithm>
#include "linagl.h"
#include "ray.h"

NAMESPACE_BEGIN

struct AABB {
	Vec3 minPoint, maxPoint; // axis aligned bounding box
	AABB() {
		minPoint = Vec3(Inf, Inf, Inf);
		maxPoint = Vec3(-Inf, -Inf, -Inf);
	}
	AABB(const Vec3 &p0, const Vec3 &p1) {
		minPoint = Vec3(std::min(p0.x, p1.x), std::min(p0.y, p1.y), std::min(p0.z, p1.z));
		maxPoint = Vec3(std::max(p0.x, p1.x), std::max(p0.y, p1.y), std::max(p0.z, p1.z));
	}
	bool Intersect(const Ray& ray, real* hitt0, real* hitt1) const;

	FORCE_INLINE void Fit(const Vec3 &p)
	{
		if (p.x < minPoint.x) minPoint.x = p.x; // min
		if (p.y < minPoint.y) minPoint.y = p.y; // min
		if (p.z < minPoint.z) minPoint.z = p.z; // min
		maxPoint.x = std::max(p.x, maxPoint.x);
		maxPoint.y = std::max(p.y, maxPoint.y);
		maxPoint.z = std::max(p.z, maxPoint.z);
	}
	FORCE_INLINE void Reset() {
		minPoint = Vec3(Inf, Inf, Inf);
		maxPoint = Vec3(-Inf, -Inf, -Inf);
	}

	FORCE_INLINE Vec3 Diagonal() const { return maxPoint - minPoint; }

	FORCE_INLINE real SurfaceArea() const {
		Vec3 d = Diagonal();
		return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
	}

	FORCE_INLINE real Volume() const {
		Vec3 d = Diagonal();
		return d.x * d.y * d.z;
	}

	FORCE_INLINE real MaximumExtent() const {
		Vec3 d = Diagonal();
		if (d.x > d.y&& d.x > d.z)
			return 0;
		else if (d.y > d.z)
			return 1;
		else
			return 2;
	}
};

FORCE_INLINE std::ostream& operator<<(std::ostream &os, const AABB &aabb) {
	os << "AABB: \n" << "minPoint: " << aabb.minPoint << std::endl << "maxPoint: " << aabb.maxPoint;
	return os;
}

AABB Union(const AABB& a, const AABB& b);
AABB Union(const AABB& a, const Vec3& p);
NAMESPACE_END