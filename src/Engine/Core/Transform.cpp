#include "Core/Transform.h"

namespace Transform
{
struct Values
{
    glm::vec3 Position;
    glm::quat Orientation;
    glm::vec3 Scale;
    Values()
        : Position()
        , Orientation()
        , Scale(1.f)
    {}
};

std::unordered_map<EntityWrapper, Values> Cache;
}

glm::mat4 Transform::AbsoluteTransformation(EntityWrapper entity)
{
    glm::mat4 t = glm::mat4(1.f);

    while (entity.Valid()) {
        t = glm::translate((const glm::vec3&)entity["Transform"]["Position"]) * glm::toMat4(glm::quat((const glm::vec3&)entity["Transform"]["Orientation"])) * glm::scale((const glm::vec3&)entity["Transform"]["Scale"]) * t;
        entity = entity.Parent();
    }

    return t;
}

glm::vec3 Transform::AbsolutePosition(World* world, EntityID entity)
{
    return Transform::AbsolutePosition(EntityWrapper(world, entity));
}

glm::vec3 Transform::AbsolutePosition(EntityWrapper entity)
{
    glm::vec3 position;
    if (!entity.Valid()) {
        return position;
    }

    ComponentWrapper entityTransform = entity["Transform"];
    if (!entityTransform["Position"].Dirty(DirtySet::Transform)) {
        return Cache[entity].Position;
    }

    EntityWrapper e = entity;
    while (e.Valid()) {
        ComponentWrapper transform = e["Transform"];
        EntityWrapper parent = e.Parent();
        position += Transform::AbsoluteOrientation(parent) * (glm::vec3)transform["Position"];
        e = parent;
    }

    Cache[entity].Position = position;
    entityTransform["Position"].SetDirty(false);

    auto dirtyChildren = entity.ChildrenWithComponent("Transform");
    for (auto& child : dirtyChildren) {
        child["Transform"]["Position"].SetDirty(true);
    }
    return position;
}

glm::vec3 Transform::AbsoluteOrientationEuler(EntityWrapper entity)
{
    glm::vec3 orientation;

    while (entity.Valid()) {
        ComponentWrapper transform = entity["Transform"];
        orientation += (Field<glm::vec3>)transform["Orientation"];
        entity = entity.Parent();
    }

    return orientation;
}

glm::quat Transform::AbsoluteOrientation(World* world, EntityID entity)
{
    return Transform::AbsoluteOrientation(EntityWrapper(world, entity));
}

glm::quat Transform::AbsoluteOrientation(EntityWrapper entity)
{
    glm::quat orientation;
    if (!entity.Valid()) {
        return orientation;
    }
    ComponentWrapper entityTransform = entity["Transform"];
    if (!entityTransform["Orientation"].Dirty(DirtySet::Transform)) {
        return Cache[entity].Orientation;
    }

    EntityWrapper e = entity;
    while (e.Valid()) {
        ComponentWrapper transform = e["Transform"];
        orientation = glm::quat((glm::vec3)transform["Orientation"]) * orientation;
        e = e.Parent();
    }

    Cache[entity].Orientation = orientation;
    entityTransform["Orientation"].SetDirty(false);

    auto dirtyChildren = entity.ChildrenWithComponent("Transform");
    for (auto& child : dirtyChildren) {
        child["Transform"]["Orientation"].SetDirty(true);
    }
    return orientation;
}

glm::vec3 Transform::AbsoluteScale(World* world, EntityID entity)
{
    return Transform::AbsoluteScale(EntityWrapper(world, entity));
}

glm::vec3 Transform::AbsoluteScale(EntityWrapper entity)
{
    glm::vec3 scale(1.f);
    if (!entity.Valid()) {
        return scale;
    }

    ComponentWrapper entityTransform = entity["Transform"];
    if (!entityTransform["Scale"].Dirty(DirtySet::Transform)) {
        return Cache[entity].Scale;
    }

    EntityWrapper e = entity;
    while (e.Valid()) {
        ComponentWrapper transform = e["Transform"];
        scale *= (glm::vec3)transform["Scale"];
        e = e.Parent();
    }

    Cache[entity].Scale = scale;
    entityTransform["Scale"].SetDirty(false);

    auto dirtyChildren = entity.ChildrenWithComponent("Transform");
    for (auto& child : dirtyChildren) {
        child["Transform"]["Scale"].SetDirty(true);
    }
    return scale;
}

glm::mat4 Transform::ModelMatrix(EntityID entity, World* world)
{
    return AbsoluteTransformation(EntityWrapper(world, entity));
}

glm::mat4 Transform::ModelMatrix(EntityWrapper entity)
{
    return AbsoluteTransformation(entity);
}

glm::vec3 Transform::TransformPoint(const glm::vec3& point, const glm::mat4& matrix)
{
    return glm::vec3(matrix * glm::vec4(point.x, point.y, point.z, 1));
}