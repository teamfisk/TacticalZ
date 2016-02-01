#ifndef Transform_h__
#define Transform_h__

#include "../GLM.h"
#include "World.h"
#include "EntityWrapper.h"

namespace Transform
{

glm::mat4 AbsoluteTransformation(EntityWrapper entity);
glm::vec3 AbsolutePosition(EntityWrapper entity);
glm::vec3 AbsolutePosition(World* world, EntityID entity);
glm::vec3 AbsoluteOrientationEuler(EntityWrapper entity);
glm::quat AbsoluteOrientation(EntityWrapper entity);
glm::quat AbsoluteOrientation(World* world, EntityID entity);
glm::vec3 AbsoluteScale(EntityWrapper entity);
glm::vec3 AbsoluteScale(World* world, EntityID entity);
glm::mat4 ModelMatrix(EntityWrapper entity);
glm::mat4 ModelMatrix(EntityID entity, World* world);

}

#endif