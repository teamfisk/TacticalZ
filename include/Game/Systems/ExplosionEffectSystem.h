#ifndef ExplosionEffectSystem_h__
#define ExplosionEffectSystem_h__

#include "Common.h"
#include "Core/System.h"
#include "Engine/GLM.h"

class ExplosionEffectSystem : public PureSystem
{
public:
    ExplosionEffectSystem(SystemParams params)
        : System(params)
        , PureSystem("ExplosionEffect")
    { }

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt) override;
};

#endif