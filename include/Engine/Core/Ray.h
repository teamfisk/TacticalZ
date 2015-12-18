#ifndef Ray_h__
#define Ray_h__

#include "../GLM.h"
#include "Common.h"

class Ray
{
public:
    Ray(const glm::vec3& origin, const glm::vec3& dir)
        : m_Origin(origin)
        , m_Direction(glm::normalize(dir))
    {
        DEBUG_IF(true) {
            if (glm::any(glm::isnan(m_Direction))) {
                LOG_WARNING("Ray Direction was set to the zero-vector, expect unknown side effects and/or crashes.");
            }
        }
    }
    const glm::vec3& Origin() const { return m_Origin; }
    const glm::vec3& Direction() const { return m_Direction; }
    //Sets the ray origin at parameter.
    void SetOrigin(const glm::vec3& origin) { m_Origin = origin; }
    //Normalizes the parameter and sets direction to it.
    void SetDirection(const glm::vec3& direction) { m_Direction = glm::normalize(direction); }
private:
    glm::vec3 m_Origin;
    glm::vec3 m_Direction;
};

#endif // Ray_h__
