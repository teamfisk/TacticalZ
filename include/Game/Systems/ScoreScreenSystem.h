#ifndef ScoreScreenSystem_h__
#define ScoreScreenSystem_h__

#include "../../Engine/Core/System.h"
#include "../../Engine/GLM.h"

class ScoreScreenSystem : public PureSystem
{
public:
    ScoreScreenSystem(SystemParams params)
        : System(params)
        , PureSystem("ScoreScreen")
    { }

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& capturePoint, double dt) override;
};

#endif