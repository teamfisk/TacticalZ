#ifndef PlayerHUD_h__
#define PlayerHUD_h__

#include "../../Engine/Core/System.h"
#include "../../Engine/GLM.h"
#include "../../Engine/Rendering/ESetCamera.h"
#include <imgui/imgui.h>

class PlayerHUD : public ImpureSystem
{
public:
    PlayerHUD(World* world, EventBroker* eventBrokerer);
    ~PlayerHUD();

    virtual void Update(double dt) override;

private:
    World* m_World;
    EventBroker* m_EventBroker;

};

#endif