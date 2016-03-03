#include "Game/Systems/AbilityCooldownHUDSystem.h"

void AbilityCooldownHUDSystem::Update(double dt)
{
    //HUD element for tracking cooldown on the parent entity with Dashability TODO: Make sure it support other abilities when they are made.

    auto abilityHUDs = m_World->GetComponents("AbilityCooldownHUD");
    if (abilityHUDs == nullptr)
        return;

    for (auto& abilityHUDC : *abilityHUDs) {
        EntityWrapper entity = EntityWrapper(m_World, abilityHUDC.EntityID);
        EntityWrapper abilityEntity = entity.FirstParentWithComponent("DashAbility");
        if (!abilityEntity.Valid())
            return;
        EntityWrapper cooldownTextEntity = entity.FirstChildByName("Cooldown");
        if(cooldownTextEntity.Valid()) {
            if(cooldownTextEntity.HasComponent("Text"))
            {
                double maxAbilityCD = (double)abilityEntity["DashAbility"]["CoolDownMaxTimer"];
                double currentAbilityCD = (double)abilityEntity["DashAbility"]["CoolDownTimer"];
                currentAbilityCD = currentAbilityCD >= 0.0 ? currentAbilityCD : 0.0;
                std::string t = (std::string&)cooldownTextEntity["Text"]["Content"] = std::to_string(currentAbilityCD).substr(0, 3);
                if(entity.HasComponent("Fill")) {
                    entity["Fill"]["Percentage"] = currentAbilityCD/maxAbilityCD; //TODO: current time needs to be in the component.
                }
            }
        }
    }
}
