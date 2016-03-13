#ifndef FadeSystem_h__
#define FadeSystem_h__

#include "Core/System.h"
#include "GLM.h"

class FadeSystem : public PureSystem
{
public:
    FadeSystem(SystemParams params)
        : System(params)
        , PureSystem("Fade")
    {

    }

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cFadeOut, double dt) override;

private:
};

#endif