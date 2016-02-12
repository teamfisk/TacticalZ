#ifndef AmmunitionHUDSystem_h__
#define AmmunitionHUDSystem_h__

#include "../../Engine/Core/System.h"
#include "../../Engine/GLM.h"

class AmmunitionHUDSystem : public ImpureSystem
{
public:
    AmmunitionHUDSystem(SystemParams params)
        : System(params)
    { }

    virtual void Update(double dt) override;
};

#endif