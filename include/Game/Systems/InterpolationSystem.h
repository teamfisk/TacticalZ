#ifndef Systems_InterpolationSystem_h__
#define Systems_InterpolationSystem_h__

#include <queue>
#include <unordered_map>
#include <boost/shared_array.hpp>
#include <glm/common.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Common.h"
#include "Core/System.h"
#include "Core/EventBroker.h"
#include "Core/ResourceManager.h"
#include "Core/ConfigFile.h"
#include "Core/EPlayerSpawned.h"

#include "Network/EInterpolate.h"

class InterpolationSystem : public PureSystem
{
public:
    InterpolationSystem(SystemParams params);
    ~InterpolationSystem() { }

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& transform, double dt) override;

private:
    struct Transform 
    {
        glm::vec3 Position;
        glm::vec3 Scale;
        glm::quat Orientation;
        float interpolationTime;
    };

    std::unordered_map<EntityID, Transform> m_NextTransform;
    std::unordered_map<EntityID, Transform> m_LastReceivedTransform;
    EntityWrapper m_LocalPlayer = EntityWrapper::Invalid;

    template <typename T>
    T vectorInterpolation(T prev, T next, double currentTime)
    {
        T difference = next - prev;
        T vector = difference * (static_cast<float>(currentTime) / m_SnapshotInterval);
        return vector;
    }
    float m_SnapshotInterval;

    EventRelay<InterpolationSystem, Events::Interpolate> m_EInterpolate;
    bool InterpolationSystem::OnInterpolate(Events::Interpolate& e);
    EventRelay<InterpolationSystem, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawned(Events::PlayerSpawned& e);
};

#endif
