#ifndef AutoBlendQueue_h__
#define AutoBlendQueue_h__

#include "../Core/ResourceManager.h"
#include "Skeleton.h"
#include "Model.h"
#include "BlendTree.h"

class AutoBlendQueue
{
public:
    struct AutoBlendJob
    {
        EntityWrapper RootNode = EntityWrapper::Invalid;
        double Duration;
        double CurrentTime = 0.0;
        double Delay = 0.0;
        BlendTree::AutoBlendInfo BlendInfo;
    };

private:
    std::map<double, AutoBlendJob> m_BlendQueue;
    

};

#endif
