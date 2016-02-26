#ifndef AnimationSystem_h__
#define AnimationSystem_h__

#include "GLM.h"

#include "Common.h"
#include "Core/System.h"
#include "Core/ResourceManager.h"
#include "Rendering/Model.h"
#include "Rendering/EAnimationComplete.h"
#include "Rendering/Skeleton.h"
#include "Rendering/BlendTree.h"

class AnimationSystem : public ImpureSystem
{
public:
    AnimationSystem(SystemParams params);
    ~AnimationSystem() { }
    virtual void Update(double dt) override;
private:
    void CreateBlendTrees();
    void UpdateAnimations(double dt);

};

#endif