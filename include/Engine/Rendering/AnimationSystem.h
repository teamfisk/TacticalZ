#ifndef AnimationSystem_h__
#define AnimationSystem_h__

#include "GLM.h"

#include "../Common.h"
#include "../Core/System.h"
#include "../Core/ResourceManager.h"
#include "Rendering/Model.h"
#include "Rendering/Skeleton.h"
#include "Rendering/BlendTree.h"
#include "Rendering/EAutoAnimationBlend.h"
#include "../Core/EntityWrapper.h"
#include "Rendering/AutoBlendQueue.h"
#include "../Input/EInputCommand.h"
#include "../Core/EEntityDeleted.h"
#include "Rendering/ESetBlendWeight.h"
#include "imgui/imgui.h"
#include "Input/EInputCommand.h"

class AnimationSystem : public ImpureSystem
{
public:
    AnimationSystem(SystemParams params);
    ~AnimationSystem() { }
    virtual void Update(double dt) override;
private:
    void CreateBlendTrees();
    void UpdateAnimations(double dt);
    void UpdateWeights(double dt);

    EventRelay<AnimationSystem, Events::AutoAnimationBlend> m_EAutoAnimationBlend;
    bool OnAutoAnimationBlend(Events::AutoAnimationBlend& e);

    EventRelay<AnimationSystem, Events::EntityDeleted> m_EEntityDeleted;
    bool OnEntityDeleted(Events::EntityDeleted& e);

    EventRelay<AnimationSystem, Events::SetBlendWeight> m_ESetBlendWeight;
    bool OnSetBlendWeight(Events::SetBlendWeight& e);

    EventRelay<AnimationSystem, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(Events::InputCommand& e);

    std::unordered_map<EntityWrapper, AutoBlendQueue> m_AutoBlendQueues;

    bool Crouch = false;
    float aimMove = 0;
};

#endif