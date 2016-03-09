#ifndef StartSystem_h__
#define StartSystem_h__

#include "Core/System.h"
#include "Core/ResourceManager.h"
#include "Core/Event.h"
#include "Core/EventBroker.h"
#include "Rendering/ESetCamera.h"

class StartSystem : public ImpureSystem
{
public:
    StartSystem(SystemParams params);
    virtual void Update(double dt) override;

private:
    EntityWrapper m_activeCamera = EntityWrapper::Invalid;
    
    EventRelay<StartSystem, Events::SetCamera> m_ECameraActivated;
    bool OnCameraActivated(const Events::SetCamera& e);
};

#endif