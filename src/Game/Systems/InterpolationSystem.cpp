#include "Systems/InterpolationSystem.h"

InterpolationSystem::InterpolationSystem(SystemParams params) 
    : System(params)
    , PureSystem("Transform")
{
    ConfigFile* config = ResourceManager::Load<ConfigFile>("Config.ini");
    m_SnapshotInterval = config->Get<float>("Networking.SnapshotInterval", 0.05f);
    EVENT_SUBSCRIBE_MEMBER(m_EInterpolate, &InterpolationSystem::OnInterpolate);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &InterpolationSystem::OnPlayerSpawned);
}

void InterpolationSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& transform, double dt)
{
    // Don't interpolate entities that might already have been removed
    if (!entity.Valid()) {
        return;
    }

    //return;
    if (m_NextTransform.find(transform.EntityID) != m_NextTransform.end()) { // Exists in map
        m_NextTransform[transform.EntityID].interpolationTime += static_cast<float>(dt);
        Transform sTransform = m_NextTransform[transform.EntityID];
        float time = sTransform.interpolationTime;
        if (time > m_SnapshotInterval) {
            if (m_LastReceivedTransform.find(transform.EntityID) != m_LastReceivedTransform.end()) {
                m_NextTransform[transform.EntityID] = m_LastReceivedTransform[transform.EntityID];
                m_NextTransform[transform.EntityID].interpolationTime = time - m_SnapshotInterval;
                sTransform = m_NextTransform[transform.EntityID];
                m_LastReceivedTransform.erase(transform.EntityID);
            } else {
                m_NextTransform.erase(transform.EntityID);
            }
        }
        if (transform.Info.Name == "Transform") {
            // Position
            glm::vec3 nextPosition = sTransform.Position;
            glm::vec3 currentPosition = static_cast<glm::vec3>(transform["Position"]);
            (glm::vec3&)transform["Position"] += vectorInterpolation<glm::vec3>(currentPosition, nextPosition, sTransform.interpolationTime);
            // Orientation
            glm::quat nextOrientation = sTransform.Orientation;
            glm::quat currentOrientation = glm::quat(static_cast<glm::vec3>(transform["Orientation"]));
            (glm::vec3&)transform["Orientation"] = glm::eulerAngles(glm::slerp<float>(currentOrientation, nextOrientation, glm::max(sTransform.interpolationTime / m_SnapshotInterval, 1.f)));
            // Scale
            glm::vec3 nextScale = sTransform.Scale;
            glm::vec3 currentScale = static_cast<glm::vec3>(transform["Scale"]);
            (glm::vec3&)transform["Scale"] += vectorInterpolation<glm::vec3>(currentScale, nextScale, sTransform.interpolationTime);
        }
    }
}

bool InterpolationSystem::OnPlayerSpawned(Events::PlayerSpawned& e)
{
    m_LocalPlayer = e.Player;
    return true;
}

bool InterpolationSystem::OnInterpolate(Events::Interpolate& e)
{
    // TODO: Make this work for arbitrary component types
    if (e.Component.Info.Name == "Transform") {
        Transform transform;
        transform.Position = e.Component["Position"];
        transform.Orientation = glm::quat((glm::vec3)e.Component["Orientation"]);
        transform.Scale = e.Component["Scale"];
        transform.interpolationTime = 0.0f;

        if (m_NextTransform.find(e.Entity.ID) != m_NextTransform.end()) { // Did exist
            m_LastReceivedTransform[e.Entity.ID] = transform;
        } else { // Did not
            m_NextTransform[e.Entity.ID] = transform;
        }
    }

    return true;
}
