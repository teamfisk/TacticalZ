#include "Core/AABB.h"
#include "Common.h"

AABB::AABB(const glm::vec3& minPos, const glm::vec3& maxPos)
    : m_MinCorner(minPos)
    , m_MaxCorner(maxPos)
    , m_Center(0.5f * (maxPos + minPos))
    , m_HalfSize(0.5f * (maxPos - minPos))
{
    IF_DEBUG_IS(true) {
        if (glm::any(glm::lessThan(m_MaxCorner, m_MinCorner))) {
            LOG_WARNING("AABB maxCorner coordinates are not greater than minCorner");
            m_MaxCorner.x = glm::max(m_MaxCorner.x, m_MinCorner.x);
            m_MinCorner.x = glm::min(m_MaxCorner.x, m_MinCorner.x);
            m_MaxCorner.y = glm::max(m_MaxCorner.y, m_MinCorner.y);
            m_MinCorner.y = glm::min(m_MaxCorner.y, m_MinCorner.y);
            m_MaxCorner.z = glm::max(m_MaxCorner.z, m_MinCorner.z);
            m_MinCorner.z = glm::min(m_MaxCorner.z, m_MinCorner.z);
        }
    }
}

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
