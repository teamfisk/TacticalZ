#include "Systems/BoostSystem.h"

BoostSystem::BoostSystem(SystemParams params)
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerDamage, &BoostSystem::OnPlayerDamage);
}

bool BoostSystem::OnPlayerDamage(Events::PlayerDamage& e)
{
    if (e.Victim.ID == e.Inflictor.ID) {
        return false;
    }

    if (!e.Victim.Valid() || !LocalPlayer.Valid()) {
        return false;
    }

    if (e.Victim != LocalPlayer && !e.Victim.IsChildOf(LocalPlayer)) {
        return false;
    }

    if (!e.Inflictor.Valid()) {
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


    LOG_INFO("intflictor: %s, vicitm: %s", e.Inflictor.Name().c_str(), e.Victim.Name().c_str());
    if(className == "SidearmWeapon") { // give ammo
        giveAmmo(e.Inflictor, e.Victim);
    } else { //give boost
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
    bool gaveAmmo = false;

    if (receiver.HasComponent("AssaultWeapon")) {
        int magazineSize = receiver["AssaultWeapon"]["MagazineSize"];
        int& magazineAmmo = receiver["AssaultWeapon"]["MagazineAmmo"];

        int maxAmmo = receiver["AssaultWeapon"]["MaxAmmo"];
        int& ammo = receiver["AssaultWeapon"]["Ammo"];

        if (magazineAmmo < magazineSize) {
            magazineAmmo += 1;
            gaveAmmo = true;
        } else if (ammo < maxAmmo) {
            ammo += 1;
            gaveAmmo = true;
        }
    } else if (receiver.HasComponent("DefenderWeapon")) {
        int magazineSize = receiver["DefenderWeapon"]["MagazineSize"];
        int& magazineAmmo = receiver["DefenderWeapon"]["MagazineAmmo"];

        int maxAmmo = receiver["DefenderWeapon"]["MaxAmmo"];
        int& ammo = receiver["DefenderWeapon"]["Ammo"];

        if (magazineAmmo < magazineSize) {
            magazineAmmo += 1;
            gaveAmmo = true;
        } else if (ammo < maxAmmo) {
            ammo += 1;
            gaveAmmo = true;
        }
    }


    if (gaveAmmo) {
        EntityWrapper effectSpawner = giver.FirstChildByName("AmmoShareEffectSpawner");
        if (effectSpawner.Valid()) {
            if (effectSpawner.HasComponent("Spawner")) {
                SpawnerSystem::Spawn(effectSpawner, effectSpawner);
            }
        }
    }
}
