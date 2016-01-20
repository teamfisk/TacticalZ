#ifndef AABB_h__
#define AABB_h__

#include "../GLM.h"

class AABB
{
public:
    AABB() = default;
    //No checks are made. Values in minPos must be less than values in maxPos, i.e. min.x < max.x, etc.
    AABB(const glm::vec3& minPos, const glm::vec3& maxPos);
    AABB(const glm::vec4& minPos, const glm::vec4& maxPos);
    //No checks are made. Size must consist of non-negative numbers.
    static AABB FromOriginSize(const glm::vec3& origin, const glm::vec3& size);
    virtual ~AABB();

    const glm::vec3& MinCorner() const { return m_MinCorner; }
    const glm::vec3& MaxCorner() const { return m_MaxCorner; }
    const glm::vec3& Origin() const { return m_Origin; }
    const glm::vec3 Size() const { return 2.0f * m_HalfSize; }
    const glm::vec3& HalfSize() const { return m_HalfSize; }
private:
    glm::vec3 m_MinCorner;
    glm::vec3 m_MaxCorner;
    glm::vec3 m_Origin;
    glm::vec3 m_HalfSize;
};

#endif