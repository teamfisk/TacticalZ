#include "Core/UniformScaleSystem.h"

UniformScaleSystem::UniformScaleSystem(SystemParams params) 
    : System(params)
    , PureSystem("UniformScale")
{
    EVENT_SUBSCRIBE_MEMBER(m_ESetCamera, &UniformScaleSystem::OnSetCamera);
}

void UniformScaleSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cUniformScale, double dt)
{
    if (!m_Camera.Valid()) {
        return;
    }

    float distance = glm::length((glm::vec3)entity["Transform"]["Position"] - (const glm::vec3&)m_Camera["Transform"]["Position"]);
    entity["Transform"]["Scale"] = (const glm::vec3&)cUniformScale["Scale"] * distance;
}

bool UniformScaleSystem::OnSetCamera(const Events::SetCamera& e)
{
    m_Camera = e.CameraEntity;
    return false;
}