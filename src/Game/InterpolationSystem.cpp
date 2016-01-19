#include "InterpolationSystem.h"

//void InterpolationSystem::UpdateComponent(World * world, ComponentWrapper & transform, double dt)
//{
//    if (m_InterpolationPoints[transform.EntityID].size() > 0) {
//        Transform& sTransform = m_InterpolationPoints[transform.EntityID].front();
//        sTransform.interpolationTime += dt;
//        if (sTransform.interpolationTime > 0.05) {
//            double time = std::fmod(sTransform.interpolationTime, 0.05f);
//            m_InterpolationPoints[transform.EntityID].pop();
//            if (m_InterpolationPoints[transform.EntityID].size() <= 0) {
//                return;
//            }
//            sTransform = m_InterpolationPoints[transform.EntityID].front();
//            sTransform.interpolationTime = time;
//        }
//        glm::vec3 nextPosition = sTransform.Position;
//        glm::vec3 currentPosition = static_cast<glm::vec3>(transform["Position"]);
//        transform["Position"] = vectorInterpolation(currentPosition, nextPosition, sTransform.interpolationTime);
//    }
//}

void InterpolationSystem::UpdateComponent(World * world, ComponentWrapper & transform, double dt)
{
    Transform& sTransform = m_InterpolationPoints[transform.EntityID];
    sTransform.interpolationTime += dt;
    // Position
    glm::vec3 nextPosition = sTransform.Position;
    glm::vec3 currentPosition = static_cast<glm::vec3>(transform["Position"]);
    (glm::vec3&)transform["Position"] += vectorInterpolation<glm::vec3>(currentPosition, nextPosition, sTransform.interpolationTime);
    // Orientation
    glm::quat nextOrientation = sTransform.Orientation;
    glm::quat currentOrientation = glm::quat(static_cast<glm::vec3>(transform["Orientation"]));
    (glm::vec3&)transform["Orientation"] = glm::eulerAngles(glm::slerp<float>(currentOrientation, nextOrientation, sTransform.interpolationTime / SNAPSHOTINTERVAL));
    // Scale
    glm::vec3 nextScale = sTransform.Scale;
    glm::vec3 currentScale = static_cast<glm::vec3>(transform["Scale"]);
    //glm::vec3 resize = vectorInterpolation<glm::vec3>(currentScale, nextScale, sTransform.interpolationTime);
    (glm::vec3&)transform["Scale"] += vectorInterpolation<glm::vec3>(currentScale, nextScale, sTransform.interpolationTime);
}

//glm::vec3 InterpolationSystem::vectorInterpolation(glm::vec3 prev, glm::vec3 next, double currentTime)
//{
//    glm::vec3 difference = next - prev;
//    glm::vec3 position = difference / SNAPSHOTINTERVAL * static_cast<float>(currentTime);
//    return position;
//}

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
    m_InterpolationPoints[e.Entity] = transform;
    // Check if queue already exists
    //if (m_InterpolationPoints.find(e.Entity) != m_InterpolationPoints.end()) { // Did exist, push to queue
    //    m_InterpolationPoints[e.Entity].push(transform);
    //} 
    
    //else { // Did not exist, create queue
    //    std::queue<Transform> transformQueue;
    //    transformQueue.push(transform);
    //    m_InterpolationPoints[e.Entity] = transformQueue;
    //}
    return false;
}
