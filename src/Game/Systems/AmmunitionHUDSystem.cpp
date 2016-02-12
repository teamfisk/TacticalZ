#include "Game/Systems/AmmunitionHUDSystem.h"

void AmmunitionHUDSystem::Update(double dt)
{
    //Hud element for tracking ammunition from parent with AssaultWeapon component.Child with the name "MagazineAmmo" tracks clip ammunition.Child with the name "Ammo" tracks ammo.</xs:documentation>

    auto ammunitionHUDs = m_World->GetComponents("AmmunitionHUD");
    if (ammunitionHUDs == nullptr) {
        return;
    }

    for (auto& ammunitionHUDComponent : *ammunitionHUDs) {
        EntityWrapper entity = EntityWrapper(m_World, ammunitionHUDComponent.EntityID);

        EntityWrapper playerEntity = entity.FirstParentWithComponent("AssaultWeapon");

        if (!playerEntity.Valid()) {
            return;
        }
        

        EntityWrapper magazineAmmo = entity.FirstChildByName("MagazineAmmo");
        if(magazineAmmo.Valid()) {
            if(magazineAmmo.HasComponent("Text")) {
                (std::string&)magazineAmmo["Text"]["Content"] = std::to_string((int)playerEntity["AssaultWeapon"]["MagazineAmmo"]);
            }
        }

        EntityWrapper ammo = entity.FirstChildByName("Ammo");
        if (ammo.Valid()) {
            if (ammo.HasComponent("Text")) {
                (std::string&)ammo["Text"]["Content"] = std::to_string((int)playerEntity["AssaultWeapon"]["Ammo"]);
            }
        }
    }
}
