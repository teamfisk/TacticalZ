#include "Game/Systems/BoostIconsHUDSystem.h"

void BoostIconsHUDSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& capturePoint, double dt)
{
    EntityWrapper assaultEntity = entity.FirstChildByName("Assault");
    EntityWrapper defenderEntity = entity.FirstChildByName("Defender");
    EntityWrapper sniperEntity = entity.FirstChildByName("Sniper");

    if(assaultEntity.Valid()) {
        if (assaultEntity.HasComponent("Fill")) {
            EntityWrapper parentWithAssaultBoost = assaultEntity.FirstParentWithComponent("BoostAssault");
            if (parentWithAssaultBoost.Valid()) {
                assaultEntity["Fill"]["Percentage"] = 1.0;
            } else {
                assaultEntity["Fill"]["Percentage"] = 0.0;
            }
        }
    }

    if (defenderEntity.Valid()) {
        if (defenderEntity.HasComponent("Fill")) {
            EntityWrapper parentWithAssaultBoost = defenderEntity.FirstParentWithComponent("BoostDefender");
            if (parentWithAssaultBoost.Valid()) {
                defenderEntity["Fill"]["Percentage"] = 1.0;
            } else {
                defenderEntity["Fill"]["Percentage"] = 0.0;
            }
        }
    }

    if (sniperEntity.Valid()) {
        if (sniperEntity.HasComponent("Fill")) {
            EntityWrapper parentWithAssaultBoost = sniperEntity.FirstParentWithComponent("BoostSniper");
            if (parentWithAssaultBoost.Valid()) {
                sniperEntity["Fill"]["Percentage"] = 1.0;
            } else {
                sniperEntity["Fill"]["Percentage"] = 0.0;
            }
        }
    }
}

