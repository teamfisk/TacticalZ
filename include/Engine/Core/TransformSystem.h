#ifndef Transform_h__
#define Transform_h__

#include "../GLM.h"
#include "System.h"
#include "EEntityDeleted.h"

class TransformSystem : public System
{
public:
    TransformSystem(SystemParams params);

    //glm::mat4 AbsoluteTransformation(EntityWrapper entity);
    static glm::vec3 AbsolutePosition(EntityWrapper entity);
    static glm::vec3 AbsolutePosition(World* world, EntityID entity);
    static glm::vec3 AbsoluteOrientationEuler(EntityWrapper entity);
    static glm::quat AbsoluteOrientation(EntityWrapper entity);
    static glm::quat AbsoluteOrientation(World* world, EntityID entity);
    static glm::vec3 AbsoluteScale(EntityWrapper entity);
    static glm::vec3 AbsoluteScale(World* world, EntityID entity);
    static glm::mat4 ModelMatrix(EntityWrapper entity);
    static glm::mat4 ModelMatrix(EntityID entity, World* world);
    static glm::vec3 TransformPoint(const glm::vec3& point, const glm::mat4& matrix);

    static int RecalculatedPositions;
    static int RecalculatedOrientations;
    static int RecalculatedScales;

private:
    static std::unordered_map<EntityWrapper, glm::vec3> PositionCache;
    static std::unordered_map<EntityWrapper, glm::quat> OrientationCache;
    static std::unordered_map<EntityWrapper, glm::vec3> ScaleCache;
    static std::unordered_map<EntityWrapper, glm::mat4> MatrixCache;

    EventRelay<TransformSystem, Events::EntityDeleted> m_EEntityDeleted;
    bool OnEntityDeleted(const Events::EntityDeleted& e);
};

#endif