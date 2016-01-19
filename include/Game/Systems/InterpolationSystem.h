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

#include "Network/EInterpolate.h"

#define SNAPSHOTINTERVAL 0.05f

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
    InterpolationSystem(EventBroker* eventBroker)
        : System(eventBroker)
        , PureSystem("Transform")
    {
        EVENT_SUBSCRIBE_MEMBER(m_EInterpolate, &InterpolationSystem::OnInterpolate);
    }
    ~InterpolationSystem() { }
    virtual void UpdateComponent(World* world, EntityWrapper& entity, ComponentWrapper& transform, double dt) override;
private:
    //std::unordered_map<EntityID, std::queue<Transform>> m_InterpolationPoints;
    std::unordered_map<EntityID, Transform> m_InterpolationPoints;

    //glm::vec3 vectorInterpolation(glm::vec3 prev, glm::vec3 next, double currentTime);
    template <typename T>
    T vectorInterpolation(T prev, T next, double currentTime)
    {
        T difference = next - prev;
        T vector = (difference / SNAPSHOTINTERVAL) * static_cast<float>(currentTime);
        return vector;
    }

    EventRelay<InterpolationSystem, Events::Interpolate> m_EInterpolate;
    bool InterpolationSystem::OnInterpolate(const Events::Interpolate& e);
};

#endif
