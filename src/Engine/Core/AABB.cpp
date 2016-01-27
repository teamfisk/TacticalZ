#include "Core/AABB.h"
#include "Common.h"

AABB::AABB(const glm::vec3& minPos, const glm::vec3& maxPos)
    : m_MinCorner(minPos)
    , m_MaxCorner(maxPos)
    , m_Origin(0.5f * (maxPos + minPos))
    , m_HalfSize(0.5f * (maxPos - minPos))
{
    DEBUG_IF(glm::any(glm::lessThan(m_MaxCorner, m_MinCorner))) {
        LOG_WARNING("AABB maxCorner coordinates are not greater than minCorner");
        m_MaxCorner.x = glm::max(m_MaxCorner.x, m_MinCorner.x);
        m_MinCorner.x = glm::min(m_MaxCorner.x, m_MinCorner.x);
        m_MaxCorner.y = glm::max(m_MaxCorner.y, m_MinCorner.y);
        m_MinCorner.y = glm::min(m_MaxCorner.y, m_MinCorner.y);
        m_MaxCorner.z = glm::max(m_MaxCorner.z, m_MinCorner.z);
        m_MinCorner.z = glm::min(m_MaxCorner.z, m_MinCorner.z);
        m_Origin = 0.5f * (m_MaxCorner + m_MinCorner);
        m_HalfSize = 0.5f * (m_MaxCorner - m_MinCorner);
    }
}

AABB AABB::FromOriginSize(const glm::vec3& origin, const glm::vec3& size)
{
    return AABB(origin - (size/2.f), origin + (size/2.f));
}

AABB::~AABB()
{ }
