#include "Systems/InterpolationSystem.h"

InterpolationSystem::InterpolationSystem(SystemParams params) 
    : System(params)
{
    ConfigFile* config = ResourceManager::Load<ConfigFile>("Config.ini");
    m_SnapshotInterval = config->Get<float>("Networking.SnapshotInterval", 0.05f);
    EVENT_SUBSCRIBE_MEMBER(m_EInterpolate, &InterpolationSystem::OnInterpolate);
}

void InterpolationSystem::Update(double dt)
{
    // Position
    for (auto& kv : m_InterpolatePosition) {
        EntityWrapper entity = kv.first;
        if (!entity.Valid()) {
            continue;
        }
        auto& iPosition = kv.second;
        Field<glm::tvec3<float, glm::highp>> position = iPosition.Component[iPosition.Field];

        iPosition.Alpha += dt;
        float alpha = glm::min(iPosition.Alpha / m_SnapshotInterval, 1.0);
        position = iPosition.Start + ((iPosition.Goal - iPosition.Start) * alpha);
    }

    // Orientation
    for (auto& kv : m_InterpolateOrientation) {
        EntityWrapper entity = kv.first;
        if (!entity.Valid()) {
            continue;
        }
        auto& iOrientation = kv.second;
        Field<glm::vec3> orientation = iOrientation.Component[iOrientation.Field];

        iOrientation.Alpha += dt / m_SnapshotInterval;
        iOrientation.Alpha = glm::min(iOrientation.Alpha, 1.0);
        orientation = glm::eulerAngles(glm::slerp(iOrientation.Start, iOrientation.Goal, (float)iOrientation.Alpha));
    }

    // Velocity
    for (auto& kv : m_InterpolateVelocity) {
        EntityWrapper entity = kv.first;
        if (!entity.Valid()) {
            continue;
        }
        auto& iVelocity = kv.second;
        Field<glm::vec3> position = iVelocity.Component[iVelocity.Field];

        iVelocity.Alpha += dt;
        float alpha = glm::min(iVelocity.Alpha / m_SnapshotInterval, 1.0);
        position = iVelocity.Start + ((iVelocity.Goal - iVelocity.Start) * alpha);
    }
}

bool InterpolationSystem::OnInterpolate(Events::Interpolate& e)
{
    if (e.Component.Info.Name == "Transform") {
        auto cTransform = e.Entity["Transform"];

        // Position
        Interpolation<glm::vec3> iPosition(
            cTransform,
            "Position",
            cTransform["Position"],
            e.Component["Position"]
        );
        m_InterpolatePosition.erase(e.Entity);
        m_InterpolatePosition.insert(std::make_pair(e.Entity, iPosition));

        // Orientation
        Interpolation<glm::quat> iOrientation(
            cTransform,
            "Orientation",
            glm::quat((Field<glm::vec3>)cTransform["Orientation"]),
            glm::quat((Field<glm::vec3>)e.Component["Orientation"])
        );
        m_InterpolateOrientation.erase(e.Entity);
        m_InterpolateOrientation.insert(std::make_pair(e.Entity, iOrientation));
    } else if (e.Component.Info.Name == "Physics") {
        auto cPhysics = e.Entity["Physics"];
        if (!e.Entity.HasComponent("Player")) {
            return false;
        }

        // Velocity
        Interpolation<glm::vec3> iVelocity(
            cPhysics,
            "Velocity",
            cPhysics["Velocity"],
            e.Component["Velocity"]
        );
        m_InterpolateVelocity.erase(e.Entity);
        m_InterpolateVelocity.insert(std::make_pair(e.Entity, iVelocity));
    }

    return true;
}
