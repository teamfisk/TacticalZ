#ifndef KillFeedSystem_h__
#define KillFeedSystem_h__

#include "../../Engine/Core/System.h"
#include "../../Engine/GLM.h"
#include "Core/EPlayerDeath.h"

class KillFeedSystem : public ImpureSystem
{
public:
    KillFeedSystem(SystemParams params)
        : System(params)
    {
        EVENT_SUBSCRIBE_MEMBER(m_EPlayerDeath, &KillFeedSystem::OnPlayerDeath);

    }

    virtual void Update(double dt) override;

private:




    EventRelay<KillFeedSystem, Events::PlayerDeath> m_EPlayerDeath;
    bool KillFeedSystem::OnPlayerDeath(Events::PlayerDeath& e);

    struct KillFeedInfo
    {
        std::string Content;
        glm::vec4 Color;
        float TimeToLive = 5.f;
    };

    std::list<KillFeedInfo> m_DeathQueue;

};

#endif