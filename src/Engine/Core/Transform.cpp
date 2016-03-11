#include "Core/Transform.h"

static std::unordered_map<EntityWrapper, glm::mat4> MatrixCache;

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
        position += Transform::AbsoluteScale(world, parent) * (Transform::AbsoluteOrientation(world, parent) * (glm::vec3)transform["Position"]);
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

glm::mat4 Transform::ModelMatrix(EntityID entity, World* world)
{
    return ModelMatrix(EntityWrapper(world, entity));
}

glm::vec3 Transform::TransformPoint(const glm::vec3& point, const glm::mat4& matrix)
{
    return glm::vec3(matrix * glm::vec4(point.x, point.y, point.z, 1));
}

glm::mat4 Transform::ModelMatrix(EntityWrapper entity)
{
    auto cacheIt = MatrixCache.find(entity);
    if (cacheIt != MatrixCache.end()) {
        return cacheIt->second;
    }

    ComponentWrapper cTransform = entity["Transform"];
    glm::mat4 t = glm::translate((const glm::vec3&)cTransform["Position"]) * glm::toMat4(glm::quat((const glm::vec3&)cTransform["Orientation"])) * glm::scale((const glm::vec3&)cTransform["Scale"]);

    EntityWrapper parent = entity.Parent();
    if (parent.Valid()) {
        t = ModelMatrix(parent) * t;
    }

    MatrixCache[entity] = t;
    return t;
}

void Transform::ClearCache::Update(double dt)
{
    MatrixCache.clear();
}
