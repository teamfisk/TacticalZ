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

class InterpolationSystem : public ImpureSystem
{
public:
    InterpolationSystem(SystemParams params);
    ~InterpolationSystem() { }

    virtual void Update(double dt) override;

private:
    template <typename T>
    struct Interpolation
    {
        Interpolation(const ComponentWrapper& Component, const std::string& Field, const T& Start, const T& Goal)
            : Component(Component)
            , Field(Field)
            , Start(Start)
            , Goal(Goal)
        { }

        ComponentWrapper Component;
        std::string Field;
        T Start;
        T Goal;
        double Alpha = 0.0;
    };

    float m_SnapshotInterval;
    std::unordered_map<EntityWrapper, Interpolation<glm::vec3>> m_InterpolatePosition;
    std::unordered_map<EntityWrapper, Interpolation<glm::quat>> m_InterpolateOrientation;
    std::unordered_map<EntityWrapper, Interpolation<glm::vec3>> m_InterpolateVelocity;

    EventRelay<InterpolationSystem, Events::Interpolate> m_EInterpolate;
    bool InterpolationSystem::OnInterpolate(Events::Interpolate& e);

    template <typename T>
    T vectorInterpolation(T prev, T next, double currentTime)
    {
        T difference = next - prev;
        T vector = difference * (static_cast<float>(currentTime) / m_SnapshotInterval);
        return vector;
    }
};

#endif
