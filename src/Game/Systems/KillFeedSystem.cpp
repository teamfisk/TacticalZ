#include "Game/Systems/KillFeedSystem.h"

void KillFeedSystem::Update(double dt)
{
    auto killFeeds = m_World->GetComponents("KillFeed");
    if (killFeeds == nullptr) {
        return;
    }

    for (auto it = m_DeathQueue.begin(); it != m_DeathQueue.end(); it++) {
        (*it).TimeToLive -= dt;
    }

    for (auto& killFeedComponent : *killFeeds) {
        EntityWrapper entity = EntityWrapper(m_World, killFeedComponent.EntityID);

        for (int i = 1; i <= 3; i++) {
            EntityWrapper child = entity.FirstChildByName("KillFeed" + std::to_string(i));
            if (child.HasComponent("Text")) {
                (Field<std::string>)child["Text"]["Content"] = "";
            }
        }
      
        int feedIndex = 1;
        for (auto it = m_DeathQueue.begin(); it != m_DeathQueue.end(); ) {
            bool remove = false;
            
            EntityWrapper child = entity.FirstChildByName("KillFeed" + std::to_string(feedIndex));

            if (child.HasComponent("Text")) {

                std::string str = "";
                //Add killer to the start of string.
                str = (*it).KillerColor + "\\" + std::to_string((*it).KillerClass) + "\\CFFFFFF" +  " " + (*it).KillerName + "  ";
                //Add Weapon to middle of string.
                str += "\\" + std::to_string((*it).KillerClass+3) + "  ";
                //Add victim to the end of string
                str += (*it).VictimColor + "\\" + std::to_string((*it).VictimClass) + "\\CFFFFFF" + " " + (*it).VictimName;

                (Field<std::string>)child["Text"]["Content"] = str;

                if ((*it).TimeToLive <= 0.f) {
                    (Field<std::string>)child["Text"]["Content"] = "";
                    remove = true;
                }
            }
            feedIndex++;
            if(feedIndex > 3) {
                break;
            }

            if(remove) {
                it = m_DeathQueue.erase(it);
            } else {
                it++;
            }
        }
    }
}

bool KillFeedSystem::OnPlayerKillDeath(Events::KillDeath& e)
{
    KillFeedInfo info;

    info.KillerName = e.KillerName;
    info.KillerID = e.Killer;
    info.KillerClass = e.KillerClass;
    info.KillerTeam = e.KillerTeam;
    if(info.KillerTeam == 2){
        info.KillerColor = m_BlueColor;
    } else if (info.KillerTeam == 3) {
        info.KillerColor = m_RedColor;
    }

    info.VictimName = e.CasualtyName;
    info.VictimID = e.Casualty;
    info.VictimClass = e.CasualtyClass;
    info.VictimTeam = e.CasualtyTeam;
    if (info.VictimTeam == 2) {
        info.VictimColor = m_BlueColor;
    } else if (info.VictimTeam == 3) {
        info.VictimColor = m_RedColor;
    }

    m_DeathQueue.push_back(info);
    return 1;
}
