#ifndef KillFeedSystem_h__
#define KillFeedSystem_h__

#include "../../Engine/Core/System.h"
#include "../../Engine/GLM.h"
#include "Network/EKillDeath.h"

class KillFeedSystem : public ImpureSystem
{
public:
    KillFeedSystem(SystemParams params)
        : System(params)
    {
        EVENT_SUBSCRIBE_MEMBER(m_EKillDeath, &KillFeedSystem::OnPlayerKillDeath)
    }

    virtual void Update(double dt) override;

private:




    EventRelay<KillFeedSystem, Events::KillDeath> m_EKillDeath;
    bool KillFeedSystem::OnPlayerKillDeath(Events::KillDeath& e);

    struct KillFeedInfo
    {
        std::string KillerName = "";
        int KillerClass = 0;
        int KillerID = -1;
        int KillerTeam = 0;
        std::string KillerColor = "";

        std::string VictimName = "";
        int VictimClass = 0;
        int VictimID = -1;
        int VictimTeam = 0;
        std::string VictimColor = "";

        bool redused;
        float TimeToLive = 5.f;
    };

    std::string m_RedColor = "\\C08366D";
    std::string m_BlueColor = "\\C6A1208";

    std::list<KillFeedInfo> m_DeathQueue;

};

#endif