#include "Core/AABB.h"

AABB::AABB(const glm::vec3& minPos, const glm::vec3& maxPos)
    : m_MinCorner(minPos)
    , m_MaxCorner(maxPos)
    , m_Center(0.5f * (maxPos + minPos))
    , m_HalfSize(0.5f * (maxPos - minPos))
{}

AABB::~AABB()
{}
