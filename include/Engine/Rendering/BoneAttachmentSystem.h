#ifndef BoneAttachmentSystem_h__
#define BoneAttachmentSystem_h__

#include "GLM.h"

#include "Common.h"
#include "Core/System.h"
#include "Core/ResourceManager.h"
#include "Rendering/Model.h"
#include "Rendering/Skeleton.h"

//Needs to be a higher orderlevel than AnimationSystem
class BoneAttachmentSystem : public PureSystem
{
public:
    BoneAttachmentSystem(SystemParams params)
        : System(params)
        , PureSystem("BoneAttachment")
    {

    }
    ~BoneAttachmentSystem() { }
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& BoneAttachmentComponent, double dt) override;
private:


};

#endif