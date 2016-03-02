#ifndef BoostIconsHUDSystem_h__
#define BoostIconsHUDSystem_h__

#include "../../Engine/Core/System.h"
#include "../../Engine/GLM.h"

class BoostIconsHUDSystem : public ImpureSystem
{
public:
    BoostIconsHUDSystem(SystemParams params)
        : System(params)
    { }

    virtual void Update(double dt) override;
};

#endif