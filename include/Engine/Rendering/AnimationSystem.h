#ifndef AnimationSystem_h__
#define AnimationSystem_h__

#include "GLM.h"

#include "Common.h"
#include "Core/System.h"
#include "Core/ResourceManager.h"
#include "Rendering/Model.h"
#include "Rendering/EAnimationComplete.h"
#include "Rendering/Skeleton.h"
#include <imgui/imgui.h>

class AnimationSystem : public PureSystem
{
public:
    AnimationSystem(World* world, EventBroker* eventBroker)
        : System(world, eventBroker)
        , PureSystem("Animation")
    {

    }
    ~AnimationSystem() { }
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& animationComponent, double dt) override;
private:

};

#endif