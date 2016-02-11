#ifndef PlayerHUD_h__
#define PlayerHUD_h__

#include "../../Engine/Core/System.h"
#include "../../Engine/GLM.h"
#include "../../Engine/Rendering/ESetCamera.h"
#include <imgui/imgui.h>

class PlayerHUDSystem : public ImpureSystem
{
public:
    PlayerHUDSystem(SystemParams params)
        : System(params)
    { }

    virtual void Update(double dt) override;
};

#endif