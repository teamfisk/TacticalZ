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
        std::string abilityName = "";

        if (!abilityEntity.Valid()) {
            //If we dont have a dash ability on player, we check for Sprint ability
            abilityEntity = entity.FirstParentWithComponent("SprintAbility");

            if (!abilityEntity.Valid()) {
                //If we dont have a sprint ability on player, we check for shield ability
                abilityEntity = entity.FirstParentWithComponent("ShieldAbility");

                if (!abilityEntity.Valid()) {
                    //If we dont have a shield ability, we return, since we cannot do anything.
                    return;
                } else {
                    //If we have a shield ability, we set the right icon
                    abilityName = "ShieldAbility";
                    if (entity.HasComponent("Sprite")) {
                        (std::string&)entity["Sprite"]["DiffuseTexture"] = "Textures\\Icons\\Abilities\\SheildDots-01.png";
                    }
                }
            } else {
                //If we do have a sprint ability, we change the icon
                abilityName = "SprintAbility";
                if (entity.HasComponent("Sprite")) {
                    (std::string&)entity["Sprite"]["DiffuseTexture"] = "Textures\\Icons\\Abilities\\Dash-01.png";
                }
            }
        } else {
            //If we have a dash ability, we set the icon to the correct one.
            abilityName = "DashAbility";
            if (entity.HasComponent("Sprite")) {
                (std::string&)entity["Sprite"]["DiffuseTexture"] = "Textures\\Icons\\Abilities\\Superman-01.png";
            }
        }

        EntityWrapper cooldownTextEntity = entity.FirstChildByName("Cooldown");

        double maxAbilityCD = (double)abilityEntity[abilityName]["CoolDownMaxTimer"];
        double currentAbilityCD = (double)abilityEntity[abilityName]["CoolDownTimer"];
        currentAbilityCD = currentAbilityCD >= 0.0 ? currentAbilityCD : 0.0;

        if (cooldownTextEntity.Valid()) {
            if (cooldownTextEntity.HasComponent("Text")) {
                std::string t = (std::string&)cooldownTextEntity["Text"]["Content"] = std::to_string(currentAbilityCD).substr(0, 3);
            }
        }

        if (entity.HasComponent("Fill")) {
            entity["Fill"]["Percentage"] = currentAbilityCD/maxAbilityCD;
        }
    }
}
