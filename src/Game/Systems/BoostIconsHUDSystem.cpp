#include "Game/Systems/BoostIconsHUDSystem.h"

void BoostIconsHUDSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& capturePoint, double dt)
{
    EntityWrapper assaultEntity = entity.FirstChildByName("Assault");
    EntityWrapper defenderEntity = entity.FirstChildByName("Defender");
    EntityWrapper sniperEntity = entity.FirstChildByName("Sniper");
    EntityWrapper player = entity.FirstParentWithComponent("Player");


    if(assaultEntity.Valid()) {
        if (assaultEntity.HasComponent("Fill")) {
            EntityWrapper assaultBoost = player.FirstChildByName("BoostAssault");
            if (assaultBoost.Valid()) {
                (Field<double>)assaultEntity["Fill"]["Percentage"] = 1.0;
            } else {
                (Field<double>)assaultEntity["Fill"]["Percentage"] = 0.0;
            }
        }
    }

    if (defenderEntity.Valid()) {
        if (defenderEntity.HasComponent("Fill")) {
            EntityWrapper defenderBoost = player.FirstChildByName("BoostDefender");
            if (defenderBoost.Valid()) {
                (Field<double>)defenderEntity["Fill"]["Percentage"] = 1.0;
            } else {
                (Field<double>)defenderEntity["Fill"]["Percentage"] = 0.0;
            }
        }
    }

    if (sniperEntity.Valid()) {
        if (sniperEntity.HasComponent("Fill")) {
            EntityWrapper sniperBoost = player.FirstChildByName("BoostSniper");
            if (sniperBoost.Valid()) {
                (Field<double>)sniperEntity["Fill"]["Percentage"] = 1.0;
            } else {
                (Field<double>)sniperEntity["Fill"]["Percentage"] = 0.0;
            }
        }
    }
}

