#ifndef AnimationSystem_h__
#define AnimationSystem_h__

#include "GLM.h"

#include "Common.h"
#include "Core/System.h"
#include "Core/ResourceManager.h"
#include "Rendering/Model.h"
#include "Rendering/EAnimationComplete.h"
#include "Rendering/Skeleton.h"

class AnimationSystem : public PureSystem
{
public:
    AnimationSystem(SystemParams params)
        : System(params)
        , PureSystem("Animation")
    {

    }
    ~AnimationSystem() { }
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& animationComponent, double dt) override;
private:


};

#endif