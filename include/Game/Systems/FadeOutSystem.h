#ifndef FadeOutSystem_h__
#define FadeOutSystem_h__

#include "Core/System.h"

class FadeOutSystem : public PureSystem
{
public:
    FadeOutSystem(SystemParams params)
        : System(params)
        , PureSystem("FadeOut")
    {

    }

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cFadeOut, double dt) override;

private:
};

#endif