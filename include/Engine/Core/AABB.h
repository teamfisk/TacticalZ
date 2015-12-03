#ifndef AABB_h__
#define AABB_h__

#include "../GLM.h"

class AABB
{
public:
    AABB() = default;
    AABB(const glm::vec3& minPos, const glm::vec3& maxPos);
    virtual ~AABB();

    const glm::vec3& MinCorner() const { return m_MinCorner; }
    const glm::vec3& MaxCorner() const { return m_MaxCorner; }
    const glm::vec3& Center() const { return m_Center; }
    const glm::vec3& HalfSize() const { return m_HalfSize; }
private:
    glm::vec3 m_MinCorner;
    glm::vec3 m_MaxCorner;
    glm::vec3 m_Center;
    glm::vec3 m_HalfSize;
};

#endif