#include "InterpolationSystem.h"

void InterpolationSystem::UpdateComponent(World * world, ComponentWrapper & transform, double dt)
{
    if (m_InterpolationPoints[transform.EntityID].size() > 0) {
        Transform& sTransform = m_InterpolationPoints[transform.EntityID].front();
        sTransform.interpolationTime += dt;
        if (sTransform.interpolationTime > 0.05) {
            double time = std::fmod(sTransform.interpolationTime, 0.05f);
            m_InterpolationPoints[transform.EntityID].pop();
            if (m_InterpolationPoints[transform.EntityID].size() <= 0) {
                return;
            }
            sTransform = m_InterpolationPoints[transform.EntityID].front();
            sTransform.interpolationTime = time;
        }
        glm::vec3 nextPosition = sTransform.Position;
        glm::vec3 currentPosition = static_cast<glm::vec3>(transform["Position"]);
        transform["Position"] = vectorInterpolation(currentPosition, nextPosition, sTransform.interpolationTime);
    }
}

glm::vec3 InterpolationSystem::vectorInterpolation(glm::vec3 prev, glm::vec3 next, double currentTime)
{
    glm::vec3 difference = next - prev;
    glm::vec3 position = difference / 0.05f * static_cast<float>(currentTime);
    return position;
}

bool InterpolationSystem::OnInterpolate(const Events::Interpolate & e)
{
    Transform transform;
    int offset = 0;
    // Read the data
    memcpy(&transform.Position, e.DataArray.get() + offset, sizeof(glm::vec3));
    offset += sizeof(glm::vec3);
    memcpy(&transform.Orientation, e.DataArray.get() + offset, sizeof(glm::vec3));
    offset += sizeof(glm::vec3);
    memcpy(&transform.Scale, e.DataArray.get() + offset, sizeof(glm::vec3));

    // Check if queue already exists
    if (m_InterpolationPoints.find(e.Entity) != m_InterpolationPoints.end()) { // Did exist, push to queue
        m_InterpolationPoints[e.Entity].push(transform);
    } else { // Did not exist, create queue
        std::queue<Transform> transformQueue;
        transformQueue.push(transform);
        m_InterpolationPoints[e.Entity] = transformQueue;
    }
    return false;
}
