#include "Systems/InterpolationSystem.h"

InterpolationSystem::InterpolationSystem(World* world, EventBroker* eventBroker) 
    : System(world, eventBroker)
    , PureSystem("Transform")
{
    ConfigFile* config = ResourceManager::Load<ConfigFile>("Config.ini");
    m_SnapshotInterval = config->Get<float>("Networking.SnapshotInterval", 0.05);
    EVENT_SUBSCRIBE_MEMBER(m_EInterpolate, &InterpolationSystem::OnInterpolate);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &InterpolationSystem::OnPlayerSpawned);
}

void InterpolationSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& transform, double dt)
{
    if (m_NextTransform.find(transform.EntityID) != m_NextTransform.end()) { // Exists in map
        m_NextTransform[transform.EntityID].interpolationTime += dt;
        Transform sTransform = m_NextTransform[transform.EntityID];
        double time = sTransform.interpolationTime;
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
            bool isLocalPlayer = entity == m_LocalPlayer || entity.IsChildOf(m_LocalPlayer);
            // Position
            glm::vec3 nextPosition = sTransform.Position;
            glm::vec3 currentPosition = static_cast<glm::vec3>(transform["Position"]);
            // HACK: Hardcoded tolerance value for player position desync = 1
            if (isLocalPlayer && glm::length(nextPosition - currentPosition) < 1.f) {
                return;
            }
            (glm::vec3&)transform["Position"] += vectorInterpolation<glm::vec3>(currentPosition, nextPosition, sTransform.interpolationTime);
            // Orientation
            // Don't force orientation for players
            if (!isLocalPlayer) {
                glm::quat nextOrientation = sTransform.Orientation;
                glm::quat currentOrientation = glm::quat(static_cast<glm::vec3>(transform["Orientation"]));
                (glm::vec3&)transform["Orientation"] = glm::eulerAngles(glm::slerp<float>(currentOrientation, nextOrientation, sTransform.interpolationTime / m_SnapshotInterval));
            }
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

bool InterpolationSystem::OnInterpolate(const Events::Interpolate & e)
{
    Transform transform;
    int offset = 0;
    // Read the data
    memcpy(&transform.Position, e.DataArray.get() + offset, sizeof(glm::vec3));
    offset += sizeof(glm::vec3);
    glm::vec3 tempOrientation;
    memcpy(&tempOrientation, e.DataArray.get() + offset, sizeof(glm::vec3));
    transform.Orientation = glm::quat(tempOrientation);
    offset += sizeof(glm::vec3);
    memcpy(&transform.Scale, e.DataArray.get() + offset, sizeof(glm::vec3));
    transform.interpolationTime = 0.0f;

    if (m_NextTransform.find(e.Entity) != m_NextTransform.end()) { // Did exist
        m_LastReceivedTransform[e.Entity] = transform;
    } else { // Did not
        m_NextTransform[e.Entity] = transform;
    }
    return false;
}
