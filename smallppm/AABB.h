#pragma once

#include "linagl.h"
#include <algorithm>
struct AABB {
	Vec minPoint, maxPoint; // axis aligned bounding box
	inline void fit(const Vec &p)
	{
		if (p.x < minPoint.x) minPoint.x = p.x; // min
		if (p.y < minPoint.y) minPoint.y = p.y; // min
		if (p.z < minPoint.z) minPoint.z = p.z; // min
		maxPoint.x = std::max(p.x, maxPoint.x);
		maxPoint.y = std::max(p.y, maxPoint.y);
		maxPoint.z = std::max(p.z, maxPoint.z);
	}
	inline void reset() {
		minPoint = Vec(1e20, 1e20, 1e20);
		maxPoint = Vec(-1e20, -1e20, -1e20);
	}
};