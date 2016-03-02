#ifndef AnimationSystem_h__
#define AnimationSystem_h__

#include "GLM.h"

#include "../Common.h"
#include "../Core/System.h"
#include "../Core/ResourceManager.h"
#include "Rendering/Model.h"
#include "Rendering/EAnimationComplete.h"
#include "Rendering/Skeleton.h"
#include "Rendering/BlendTree.h"
#include "Rendering/EAnimationBlend.h"
#include "Rendering/EAutoAnimationBlend.h"
#include "../Input/EInputCommand.h"
#include "../Core/EntityWrapper.h"

#include "imgui/imgui.h"

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
    void AnimationComplete(EntityWrapper animationEntity);

    EventRelay<AnimationSystem, Events::AnimationBlend> m_EAnimationBlend;
    bool OnAnimationBlend(Events::AnimationBlend& e);
    EventRelay<AnimationSystem, Events::AutoAnimationBlend> m_EAutoAnimationBlend;
    bool OnAutoAnimationBlend(Events::AutoAnimationBlend& e);


    EventRelay<AnimationSystem, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);

    struct BlendJob
    {
        EntityWrapper BlendEntity = EntityWrapper::Invalid;
        double StartWeight;
        double GoalWeight;
        double Duration;
        double CurrentTime = 0.0;
    };

    struct QueuedBlendJob : BlendJob
    {
        EntityWrapper AnimationEntity = EntityWrapper::Invalid;
    };

    struct AutoBlendJob 
    {
        EntityWrapper RootNode = EntityWrapper::Invalid;
        double Duration;
        double CurrentTime = 0.0;
        BlendTree::AutoBlendInfo BlendInfo;
    };

    std::list<AutoBlendJob> m_AutoBlendJobs;
    std::list<BlendJob> m_BlendJobs;
    std::list<QueuedBlendJob> m_QueuedBlendJobs;

    char m_AnimationName1[20] = "Run";
    float m_BlendTime1 = 0.5f;

    char m_AnimationName2[20] = "Jump";
    float m_BlendTime2 = 0.5f;
};

#endif