#include "../Game/Systems/StartSystem.h"

StartSystem::StartSystem(SystemParams params)
    : System(params)
    , ImpureSystem()
{
    EVENT_SUBSCRIBE_MEMBER(m_ECameraActivated, &StartSystem::OnCameraActivated);
}

void StartSystem::Update(double dt)
{
    auto cameras = m_World->GetComponents("Camera");
    if(cameras == nullptr) {
        return;
    }
    for(auto& cCamera: *cameras) {
        EntityWrapper cameraEntity = EntityWrapper(m_World, cCamera.EntityID);
        if(cameraEntity == m_activeCamera){
            return;
        }
        if(cameraEntity.Name() == "Overview_Camera_Start_Menu") {
            Events::SetCamera event;
            event.CameraEntity = cameraEntity;
            m_EventBroker->Publish(event);
        }
    }
}

bool StartSystem::OnCameraActivated(const Events::SetCamera& e)
{
    m_activeCamera = e.CameraEntity;
    return 1;
}
