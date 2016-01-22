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

#include "Network/EInterpolate.h"

class InterpolationSystem : public PureSystem
{
    struct Transform 
    {
        glm::vec3 Position;
        glm::vec3 Scale;
        glm::quat Orientation;
        double interpolationTime;
    };
public:
    InterpolationSystem(World* world, EventBroker* eventBroker)
        : System(world, eventBroker)
        , PureSystem("Transform")
    {
        ConfigFile* config = ResourceManager::Load<ConfigFile>("Config.ini");
        m_SnapshotInterval = config->Get<float>("Networking.SnapshotInterval", 0.05);
        EVENT_SUBSCRIBE_MEMBER(m_EInterpolate, &InterpolationSystem::OnInterpolate);
    }
    ~InterpolationSystem() { }
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& transform, double dt) override;
private:
    std::unordered_map<EntityID, Transform> m_NextTransform;
    std::unordered_map<EntityID, Transform> m_LastReceivedTransform;

    //glm::vec3 vectorInterpolation(glm::vec3 prev, glm::vec3 next, double currentTime);
    template <typename T>
    T vectorInterpolation(T prev, T next, double currentTime)
    {
        T difference = next - prev;
        T vector = (difference / m_SnapshotInterval) * static_cast<float>(currentTime);
        return vector;
    }
    float m_SnapshotInterval;

    EventRelay<InterpolationSystem, Events::Interpolate> m_EInterpolate;
    bool InterpolationSystem::OnInterpolate(const Events::Interpolate& e);
};

#endif
