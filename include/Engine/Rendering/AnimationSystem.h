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

    EventRelay<AnimationSystem, Events::AutoAnimationBlend> m_EAutoAnimationBlend;
    bool OnAutoAnimationBlend(Events::AutoAnimationBlend& e);

    std::unordered_map<EntityWrapper, AutoBlendQueue> m_AutoBlendQueues;
};

#endif