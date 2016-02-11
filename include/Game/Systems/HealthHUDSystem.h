#ifndef PlayerHUD_h__
#define PlayerHUD_h__

#include "../../Engine/Core/System.h"
#include "../../Engine/GLM.h"

class HealthHUDSystem : public ImpureSystem
{
public:
    HealthHUDSystem(SystemParams params)
        : System(params)
    { }

    virtual void Update(double dt) override;
};

#endif