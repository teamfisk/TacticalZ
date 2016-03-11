#ifndef Transform_h__
#define Transform_h__

#include "../GLM.h"
#include "World.h"
#include "EntityWrapper.h"
#include "System.h"

namespace Transform
{

glm::vec3 AbsolutePosition(EntityWrapper entity);
glm::vec3 AbsolutePosition(World* world, EntityID entity);
glm::vec3 AbsoluteOrientationEuler(EntityWrapper entity);
glm::quat AbsoluteOrientation(EntityWrapper entity);
glm::quat AbsoluteOrientation(World* world, EntityID entity);
glm::vec3 AbsoluteScale(EntityWrapper entity);
glm::vec3 AbsoluteScale(World* world, EntityID entity);
glm::mat4 ModelMatrix(EntityWrapper entity);
glm::mat4 ModelMatrix(EntityID entity, World* world);
glm::vec3 TransformPoint(const glm::vec3& point, const glm::mat4& matrix);

class ClearCache : public ImpureSystem
{
public:
    ClearCache(SystemParams params)
        : System(params)
    { }

    virtual void Update(double dt) override;
};

}

#endif