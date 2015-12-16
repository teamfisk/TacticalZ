#include "Core/AABB.h"

AABB::AABB(const glm::vec3& minPos, const glm::vec3& maxPos)
    : m_MinCorner(minPos)
    , m_MaxCorner(maxPos)
    , m_Center(0.5f * (maxPos + minPos))
    , m_HalfSize(0.5f * (maxPos - minPos))
{}

AABB::AABB(const glm::vec4& minPos, const glm::vec4& maxPos)
    : AABB(glm::vec3(minPos), glm::vec3(maxPos))
{}

void AABB::CreateFromCenter(const glm::vec3& center, const glm::vec3& size)
{
    m_Center = center;
    m_HalfSize = 0.5f * size;
    m_MinCorner = m_Center - m_HalfSize;
    m_MaxCorner = m_Center + m_HalfSize;
}

AABB::~AABB()
{}
