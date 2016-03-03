#ifndef AbilityCooldownHUDSystem_h__
#define AbilityCooldownHUDSystem_h__

#include "../../Engine/Core/System.h"
#include "../../Engine/GLM.h"

class AbilityCooldownHUDSystem : public ImpureSystem
{
public:
    AbilityCooldownHUDSystem(SystemParams params)
        : System(params)
    { }

    virtual void Update(double dt) override;
};

#endif