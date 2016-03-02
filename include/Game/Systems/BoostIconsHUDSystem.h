#ifndef BoostIconsHUDSystem_h__
#define BoostIconsHUDSystem_h__

#include "../../Engine/Core/System.h"
#include "../../Engine/GLM.h"

class BoostIconsHUDSystem : public PureSystem
{
public:
    BoostIconsHUDSystem(SystemParams params)
        : System(params)
        , PureSystem("BoostIconsHUD")
    { }

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& capturePoint, double dt) override;

};

#endif