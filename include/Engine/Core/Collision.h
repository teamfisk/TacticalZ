#ifndef Collision_h__
#define Collision_h__

#include "Core/Ray.h"
#include "Core/AABB.h"
#include <algorithm>

namespace Collision
{

bool RayAABBIntr(const Ray& ray, const AABB& box);
bool RayVsAABB(const Ray& ray, const AABB& box);
}

#endif