#ifndef AutoBlendQueue_h__
#define AutoBlendQueue_h__

#include "../Core/ResourceManager.h"
#include "Skeleton.h"
#include "Model.h"
#include "BlendTree.h"
#include "../Core/EntityWrapper.h"

class AutoBlendQueue
{
public:
    struct AutoBlendJob
    {
        EntityWrapper RootNode = EntityWrapper::Invalid;
        double Duration;
        double CurrentTime = 0.0;
        double Delay = 0.0;
        EntityWrapper AnimationEntity = EntityWrapper::Invalid;
        BlendTree::AutoBlendInfo BlendInfo;
    };

    struct AutoblendNode
    {
        AutoBlendJob BlendJob;
        double StartTime;
        double EndTime;
    };

    AutoBlendQueue() { };

    void Insert(AutoBlendJob autoBlendJob);
    void UpdateTime(double dt);

    void PrintQueue();
    bool HasActiveBlendJob();
    std::shared_ptr<BlendTree> GetBlendTree();

    AutoBlendQueue::AutoBlendJob& GetActiveBlendJob();
private:
     std::list<AutoblendNode> m_BlendQueue;
    

};

#endif
