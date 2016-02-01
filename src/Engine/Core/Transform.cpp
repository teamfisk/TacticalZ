#include "Core/Transform.h"

glm::mat4 Transform::AbsoluteTransformation(EntityWrapper entity)
{
    glm::mat4 t = glm::mat4(1.f);

    while (entity.Valid()) {
        t = glm::translate((glm::vec3&)entity["Transform"]["Position"]) * glm::toMat4(glm::quat((glm::vec3&)entity["Transform"]["Orientation"])) * glm::scale((glm::vec3&)entity["Transform"]["Scale"]) * t;
        entity = entity.Parent();
    }

    return t;
}

glm::vec3 Transform::AbsolutePosition(EntityWrapper entity)
{
    return AbsolutePosition(entity.World, entity.ID);
}

glm::vec3 Transform::AbsolutePosition(World* world, EntityID entity)
{
    glm::vec3 position;

    while (entity != EntityID_Invalid) {
        ComponentWrapper transform = world->GetComponent(entity, "Transform");
        EntityID parent = world->GetParent(entity);
        position += Transform::AbsoluteOrientation(world, parent) * (glm::vec3)transform["Position"];
        entity = parent;
    }

    return position;
}

glm::vec3 Transform::AbsoluteOrientationEuler(EntityWrapper entity)
{
    glm::vec3 orientation;

    while (entity.Valid()) {
        ComponentWrapper transform = entity["Transform"];
        orientation += (glm::vec3)transform["Orientation"];
        entity = entity.Parent();
    }

    return orientation;
}

glm::quat Transform::AbsoluteOrientation(EntityWrapper entity)
{
    return AbsoluteOrientation(entity.World, entity.ID);
}

glm::quat Transform::AbsoluteOrientation(World* world, EntityID entity)
{
    glm::quat orientation;

    while (entity != EntityID_Invalid) {
        ComponentWrapper transform = world->GetComponent(entity, "Transform");
        orientation = glm::quat((glm::vec3)transform["Orientation"]) * orientation;
        entity = world->GetParent(entity);
    }

    return orientation;
}

glm::vec3 Transform::AbsoluteScale(EntityWrapper entity)
{
    return AbsoluteScale(entity.World, entity.ID);
}

glm::vec3 Transform::AbsoluteScale(World* world, EntityID entity)
{
    glm::vec3 scale(1.f);

    while (entity != EntityID_Invalid) {
        ComponentWrapper transform = world->GetComponent(entity, "Transform");
        scale *= (glm::vec3)transform["Scale"];
        entity = world->GetParent(entity);
    }

    return scale;
}

glm::mat4 Transform::ModelMatrix(EntityWrapper entity)
{
    return ModelMatrix(entity.ID, entity.World);
}

glm::mat4 Transform::ModelMatrix(EntityID entity, World* world)
{
    return AbsoluteTransformation(EntityWrapper(world, entity));

    glm::vec3 position = Transform::AbsolutePosition(world, entity);
    glm::quat orientation = Transform::AbsoluteOrientation(world, entity);
    glm::vec3 scale = Transform::AbsoluteScale(world, entity);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(), position) * glm::toMat4(orientation) * glm::scale(scale);
    return modelMatrix;
}

