#include "Game/Systems/KillFeedSystem.h"

void KillFeedSystem::Update(double dt)
{
    auto killFeeds = m_World->GetComponents("KillFeed");
    if (killFeeds == nullptr) {
        return;
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
                (Field<std::string>)child["Text"]["Content"] = (*it).Content;
                (Field<glm::vec4>)child["Text"]["Color"] = (*it).Color;

                (*it).TimeToLive -= dt;

                if ((*it).TimeToLive <= 0.f) {
                    (Field<std::string>)child["Text"]["Content"] = "";
                    (Field<glm::vec4>)child["Text"]["Color"] = (*it).Color;
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

bool KillFeedSystem::OnPlayerDeath(Events::PlayerDeath& e)
{
    KillFeedInfo kfInfo;

    if (e.Player.HasComponent("Team")) {
        int red = e.Player["Team"].Enum("Team", "Red");
        int blue = e.Player["Team"].Enum("Team", "Blue");

        if ((int)e.Player["Team"]["Team"] == red) {
            kfInfo.Content = "Blue Player killed Red Player";
            kfInfo.Color = glm::vec4(0.f, 0.2f, 1.f, 0.8f);
            m_DeathQueue.push_back(kfInfo);
        } else if ((int)e.Player["Team"]["Team"] == blue) {
            kfInfo.Content = "Red Player killed blue Player";
            kfInfo.Color = glm::vec4(1.f, 0.f, 0.f, 0.8f);
            m_DeathQueue.push_back(kfInfo);
        }
    }


    if(m_DeathQueue.size() > 3) {
        m_DeathQueue.pop_front();
    }

    return true;
}
