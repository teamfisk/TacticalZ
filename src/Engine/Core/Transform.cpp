#include "Core/Transform.h"

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

glm::mat4 Transform::ModelMatrix(EntityID entity, World* world)
{
    glm::vec3 position = Transform::AbsolutePosition(world, entity);
    glm::quat orientation = Transform::AbsoluteOrientation(world, entity);
    glm::vec3 scale = Transform::AbsoluteScale(world, entity);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(), position) * glm::toMat4(orientation) * glm::scale(scale);
    return modelMatrix;
}

glm::vec3 Transform::TransformPoint(const glm::vec3& point, const glm::mat4& matrix)
{
    return glm::vec3(matrix * glm::vec4(point.x, point.y, point.z, 1));
}