#include "Rendering/AutoBlendQueue.h"

void AutoBlendQueue::Insert(AutoBlendJob autoBlendJob)
{
    if(!autoBlendJob.RootNode.HasComponent("Model")) {
        return;
    }

    AutoblendNode blendNode;
    blendNode.BlendJob = autoBlendJob;
    blendNode.StartTime = autoBlendJob.Delay;
    blendNode.EndTime = autoBlendJob.Delay + autoBlendJob.Duration;



    if (autoBlendJob.AnimationEntity.Valid()) {
        if (autoBlendJob.AnimationEntity.HasComponent("Animation")) {
            Model* model;
            try {
                model = ResourceManager::Load<::Model, true>((std::string)autoBlendJob.RootNode["Model"]["Resource"]);
            } catch (const std::exception&) {
                return;
            }

            Skeleton* skeleton = model->m_RawModel->m_Skeleton;
            if (skeleton == nullptr) {
                return;
            }

            const Skeleton::Animation* animation = skeleton->GetAnimation((std::string)autoBlendJob.AnimationEntity["Animation"]["AnimationName"]);

            if (animation == nullptr) {
                return;
            }

            double AnimationDuration = 0.0;

            double animationSpeed = (double)autoBlendJob.AnimationEntity["Animation"]["Speed"];
            double animationTime = (double)autoBlendJob.AnimationEntity["Animation"]["Time"];


            if ((bool)autoBlendJob.AnimationEntity["Animation"]["Reverse"]) {
                AnimationDuration = (animation->Duration * animationSpeed) - (animation->Duration - animationTime);
            } else {
                AnimationDuration = (animation->Duration * animationSpeed) - animationTime;
            }

            LOG_INFO("Animation Duration %f", AnimationDuration);
            blendNode.StartTime += AnimationDuration;
            blendNode.EndTime += AnimationDuration;
            if (m_BlendQueue.size() == 0) {
                m_BlendQueue.push_back(blendNode);
            } else {
                for (auto it = m_BlendQueue.begin(); it != m_BlendQueue.end(); it++) {
                    auto next = std::next(it, 1);


                    if (it->BlendJob.BlendInfo.NodeName == autoBlendJob.BlendInfo.NodeName) {
                        (*it) = blendNode;
                    }

                    if (next != m_BlendQueue.end()) {
                        if (it->StartTime >= blendNode.StartTime && next->StartTime <= blendNode.StartTime) {
                            LOG_INFO("Inserted %s between %s and %s", blendNode.BlendJob.BlendInfo.NodeName.c_str(), it->BlendJob.BlendInfo.NodeName.c_str(), next->BlendJob.BlendInfo.NodeName.c_str());
                            m_BlendQueue.insert(next, blendNode);
                            return;
                        }
                    } else if(it->StartTime > blendNode.StartTime){
                        LOG_INFO("Inserted %s after %s", blendNode.BlendJob.BlendInfo.NodeName.c_str(), it->BlendJob.BlendInfo.NodeName.c_str());
                        m_BlendQueue.push_front(blendNode);
                        return;
                    } else if (it->StartTime <= blendNode.StartTime) {
                        LOG_INFO("Inserted %s after %s", blendNode.BlendJob.BlendInfo.NodeName.c_str(), it->BlendJob.BlendInfo.NodeName.c_str());
                        m_BlendQueue.push_back(blendNode);
                        return;
                    }
                }
            }
        }
    }


    LOG_INFO("Cleared BlendQueue and inserted %s", blendNode.BlendJob.BlendInfo.NodeName.c_str());
    m_BlendQueue.clear();
    m_BlendQueue.push_back(blendNode);
}

void AutoBlendQueue::UpdateTime(double dt)
{
    for (auto it = m_BlendQueue.begin(); it != m_BlendQueue.end();) {
        if (it->EndTime <= 0) {
            it = m_BlendQueue.erase(it);
        } else {
            it->EndTime -= dt;
            it->StartTime -= dt;
            it++;
        }
    }
}

void AutoBlendQueue::PrintQueue()
{
    for (auto it = m_BlendQueue.begin(); it != m_BlendQueue.end(); it++) {
        LOG_INFO("Start: %f End: %f \t %s", it->StartTime, it->EndTime, it->BlendJob.BlendInfo.NodeName.c_str());
    }
}


bool AutoBlendQueue::HasActiveBlendJob()
{
    if(m_BlendQueue.empty()) {
        return false;
    } else {
        AutoblendNode blendNode = m_BlendQueue.front();
        if (blendNode.StartTime <= 0) {
            AutoBlendJob blendJob = blendNode.BlendJob;

            if (!blendJob.RootNode.Valid()) {
                m_BlendQueue.pop_front();
                return false;
            }

            if (!blendJob.RootNode.HasComponent("Model")) {
                m_BlendQueue.pop_front();
                return HasActiveBlendJob();
            }

            Model* model;
            try {
                model = ResourceManager::Load<::Model, true>(blendJob.RootNode["Model"]["Resource"]);
            } catch (const std::exception&) {
                m_BlendQueue.pop_front();
                return HasActiveBlendJob();
            }

            Skeleton* skeleton = model->m_RawModel->m_Skeleton;
            if (skeleton == nullptr) {
                m_BlendQueue.pop_front();
                return HasActiveBlendJob();
            }


            if (skeleton->BlendTrees.find(blendJob.RootNode) != skeleton->BlendTrees.end()) {
                return true;
            } else {
                m_BlendQueue.pop_front();
                return HasActiveBlendJob();
            }

            
        } else {
            return false;
        }
    }
}


std::shared_ptr<BlendTree> AutoBlendQueue::GetBlendTree()
{
    AutoblendNode blendNode = m_BlendQueue.front();
    AutoBlendJob blendJob = blendNode.BlendJob;

    if (!blendJob.RootNode.Valid()) {
        m_BlendQueue.pop_front();
        return false;
    }

    if (!blendJob.RootNode.HasComponent("Model")) {
        return nullptr;
    }

    Model* model;
    try {
        model = ResourceManager::Load<::Model, true>(blendJob.RootNode["Model"]["Resource"]);
    } catch (const std::exception&) {
        return nullptr;
    }

    Skeleton* skeleton = model->m_RawModel->m_Skeleton;
    if (skeleton == nullptr) {
        return nullptr;
    }


    if (skeleton->BlendTrees.find(blendJob.RootNode) != skeleton->BlendTrees.end()) {
        return skeleton->BlendTrees.at(blendJob.RootNode);
    } else {
        return nullptr;
    }



}

AutoBlendQueue::AutoBlendJob& AutoBlendQueue::GetActiveBlendJob()
{
    AutoblendNode& blendNode = m_BlendQueue.front();
    blendNode.BlendJob.CurrentTime = -blendNode.StartTime;
    return blendNode.BlendJob;
}
