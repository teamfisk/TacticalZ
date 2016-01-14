#ifndef Transform_h__
#define Transform_h__

#include "../GLM.h"
#include "World.h"

static class Transform
{
public:
    static glm::vec3 AbsolutePosition(World* world, EntityID entity)
    {
        glm::vec3 position;

        while (entity != EntityID_Invalid) {
            ComponentWrapper transform = world->GetComponent(entity, "Transform");
            EntityID parent = world->GetParent(entity);
            position += AbsoluteOrientation(world, parent) * (glm::vec3)transform["Position"];
            entity = parent;
        }

        return position;
    };

    static glm::quat AbsoluteOrientation(World* world, EntityID entity)
    {
        glm::quat orientation;

        while (entity != EntityID_Invalid) {
            ComponentWrapper transform = world->GetComponent(entity, "Transform");
            orientation = glm::quat((glm::vec3)transform["Orientation"]) * orientation;
            entity = world->GetParent(entity);
        }

        return orientation;
    };

    static glm::vec3 AbsoluteScale(World* world, EntityID entity)
    {
        glm::vec3 scale(1.f);

        while (entity != EntityID_Invalid) {
            ComponentWrapper transform = world->GetComponent(entity, "Transform");
            scale *= (glm::vec3)transform["Scale"];
            entity = world->GetParent(entity);
        }

        return scale;
    };

    static glm::mat4 ModelMatrix(EntityID entity, World* world)
    {
        glm::vec3 position = Transform::AbsolutePosition(world, entity);
        glm::quat orientation = Transform::AbsoluteOrientation(world, entity);
        glm::vec3 scale = Transform::AbsoluteScale(world, entity);

        glm::mat4 modelMatrix = glm::translate(glm::mat4(), position) * glm::toMat4(orientation) * glm::scale(scale);
        return modelMatrix;
    }

};

#endif