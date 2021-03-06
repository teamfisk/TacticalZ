#include "Systems/BoostSystem.h"

BoostSystem::BoostSystem(SystemParams params)
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerDamage, &BoostSystem::OnPlayerDamage);
}

bool BoostSystem::OnPlayerDamage(Events::PlayerDamage& e)
{
    

    if (e.Victim == e.Inflictor) {
        return false;
    }

    if (!e.Victim.Valid() || !e.Inflictor.Valid()) {
        return false;
    }


    //if its not friendly fire, return
    if ((int)m_World->GetComponent(e.Inflictor.ID, "Team")["Team"] != (int)m_World->GetComponent(e.Victim.ID, "Team")["Team"]) {
        return false;
    }

    //determine the inflictors class
    auto className = DetermineClass(e.Inflictor);
    if (className == "") {
        return false;
    }

    if (className == "SidearmWeapon") { // give ammo
        giveAmmo(e.Inflictor, e.Victim);
    } else { //give boost
        if (!IsServer) {
            return false;
        }
        //get the XML file, example: "Schema/Entities/BoostclassName.xml"
        std::string classXML = "Schema/Entities/" + className + ".xml";

        //check if player already has a child with the component, if so delete that child
        auto playerBoostAssaultEntity = e.Victim.FirstChildByName(className);
        if (playerBoostAssaultEntity.Valid()) {
            m_World->DeleteEntity(playerBoostAssaultEntity.ID);
        }
        //load boost XML file, set it entity parented with the victim player
        auto entityFile = ResourceManager::Load<EntityFile>(classXML);
        EntityWrapper boostAssaultEntity = entityFile->MergeInto(m_World);
        m_World->SetName(boostAssaultEntity.ID, className);
        m_World->SetParent(boostAssaultEntity.ID, e.Victim.ID);
    }


    return true;
}

std::string BoostSystem::DetermineClass(EntityWrapper inflictorPlayer)
{
    //determine the class based on what component the inflictor-player has
    if (inflictorPlayer.HasComponent("Player")) {
        if ((std::string)inflictorPlayer["Player"]["CurrentWeapon"] == "AssaultWeapon") {
            return "BoostAssault";
        }
        if ((std::string)inflictorPlayer["Player"]["CurrentWeapon"] == "DefenderWeapon") {
            return "BoostDefender";
        }
        if ((std::string)inflictorPlayer["Player"]["CurrentWeapon"] == "SniperWeapon") {
            return "BoostSniper";
        }
        if ((std::string)inflictorPlayer["Player"]["CurrentWeapon"] == "SidearmWeapon") {
            return "SidearmWeapon";
        }
    }
    return "";
}

void BoostSystem::giveAmmo(EntityWrapper giver, EntityWrapper receiver)
{
    if (receiver.HasComponent("AssaultWeapon")) {
        int magazineSize = receiver["AssaultWeapon"]["MagazineSize"];
        Field<int> magazineAmmo = receiver["AssaultWeapon"]["MagazineAmmo"];
        int prevMagazineAmmo = receiver["AssaultWeapon"]["MagazineAmmo"];
        int maxAmmo = receiver["AssaultWeapon"]["MaxAmmo"];
        Field<int> ammo = receiver["AssaultWeapon"]["Ammo"];

        int givenAmmo = 20;

        magazineAmmo = glm::clamp(magazineAmmo + givenAmmo, 0, magazineSize);
        if (magazineAmmo > prevMagazineAmmo) {
            givenAmmo = prevMagazineAmmo + givenAmmo - magazineSize;
        }
        if (givenAmmo > 0) {
            ammo = glm::clamp(ammo + givenAmmo, 0, maxAmmo);
        }
    } else if (receiver.HasComponent("DefenderWeapon")) {
        int magazineSize = receiver["DefenderWeapon"]["MagazineSize"];
        Field<int> magazineAmmo = receiver["DefenderWeapon"]["MagazineAmmo"];
        int prevMagazineAmmo = receiver["DefenderWeapon"]["MagazineAmmo"];
        int maxAmmo = receiver["DefenderWeapon"]["MaxAmmo"];
        Field<int> ammo = receiver["DefenderWeapon"]["Ammo"];

        int givenAmmo = 3;

        magazineAmmo = glm::clamp(magazineAmmo + givenAmmo, 0, magazineSize);
        if (magazineAmmo > prevMagazineAmmo) {
            givenAmmo = prevMagazineAmmo + givenAmmo - magazineSize;
        }
        if (givenAmmo > 0) {
            ammo = glm::clamp(ammo + givenAmmo, 0, maxAmmo);
        }
    }
}
