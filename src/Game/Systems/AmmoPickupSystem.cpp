#include "Systems/AmmoPickupSystem.h"

AmmoPickupSystem::AmmoPickupSystem(SystemParams params)
    : System(params)
{
    if (IsServer) {
        EVENT_SUBSCRIBE_MEMBER(m_ETriggerTouch, &AmmoPickupSystem::OnTriggerTouch);
        EVENT_SUBSCRIBE_MEMBER(m_ETriggerLeave, &AmmoPickupSystem::OnTriggerLeave);
    }
    if (IsClient) {
        EVENT_SUBSCRIBE_MEMBER(m_EAmmoPickup, &AmmoPickupSystem::OnAmmoPickup);
    }
}

void AmmoPickupSystem::Update(double dt)
{
    if (IsServer) {
        auto it = m_ETriggerTouchVector.begin();
        while (it != m_ETriggerTouchVector.end()) {
            auto& somePickup = *it;
            somePickup.DecreaseThisRespawnTimer -= dt;
            if (somePickup.DecreaseThisRespawnTimer < 0.0) {
                auto entityFile = ResourceManager::Load<EntityFile>("Schema/Entities/AmmoPickup.xml");
                EntityWrapper ammoPickup = entityFile->MergeInto(m_World);

                //let the world know a pickup has spawned
                Events::PickupSpawned ePickupSpawned;
                ePickupSpawned.Pickup = ammoPickup;
                m_EventBroker->Publish(ePickupSpawned);

                //copy values from the old entity to the new entity
                auto& newAmmoPickupEntity = ammoPickup;
                newAmmoPickupEntity["Transform"]["Position"] = somePickup.Pos;
                newAmmoPickupEntity["AmmoPickup"]["AmmoGain"] = somePickup.AmmoGain;
                newAmmoPickupEntity["AmmoPickup"]["RespawnTimer"] = somePickup.RespawnTimer;
                m_World->SetParent(newAmmoPickupEntity.ID, somePickup.parentID);

                //erase the current element (somePickup)
                it = m_ETriggerTouchVector.erase(it);
            } else {
                it++;
            }
        }
        //still touching m_PickupAtMaximum?
        for (auto& it = m_PickupAtMaximum.begin(); it != m_PickupAtMaximum.end(); ++it) {
            if (!it->player.Valid()) {
                m_PickupAtMaximum.erase(it);
                break;
            }
            if (!DoesPlayerHaveMaxAmmo(it->player)) {
                DoPickup(it->player, it->trigger);
                m_PickupAtMaximum.erase(it);
                break;
            }
        }
    }
}
bool AmmoPickupSystem::DoesPlayerHaveMaxAmmo(EntityWrapper &player) {
    PlayerClass playerClass = DetermineClass(player);
    if (playerClass == PlayerClass::Defender) {
        return !((int)player["DefenderWeapon"]["Ammo"] < (int)player["DefenderWeapon"]["MaxAmmo"]);
    } else if (playerClass == PlayerClass::Sniper) {
        return !((int)player["SniperWeapon"]["Ammo"] < (int)player["SniperWeapon"]["MaxAmmo"]);
    } else if (playerClass == PlayerClass::Assault) {
        return !((int)player["AssaultWeapon"]["Ammo"] < (int)player["AssaultWeapon"]["MaxAmmo"]);
    } else {
        return false;
    }
}
int& AmmoPickupSystem::GetPlayerAmmo(EntityWrapper &player) {
    PlayerClass playerClass = DetermineClass(player);
    if (playerClass == PlayerClass::Defender) {
        return (int)player["DefenderWeapon"]["Ammo"];
    } else if (playerClass == PlayerClass::Sniper) {
        return (int)player["SniperWeapon"]["Ammo"];
    } else if (playerClass == PlayerClass::Assault) {
        return (int)player["AssaultWeapon"]["Ammo"];
    } else {
        //TODO: should really return something here
    }
}
int AmmoPickupSystem::GetPlayerMaxAmmo(EntityWrapper &player) {
    PlayerClass playerClass = DetermineClass(player);
    if (playerClass == PlayerClass::Defender) {
        return (int)player["DefenderWeapon"]["MaxAmmo"];
    } else if (playerClass == PlayerClass::Sniper) {
        return (int)player["SniperWeapon"]["MaxAmmo"];
    } else if (playerClass == PlayerClass::Assault) {
        return (int)player["AssaultWeapon"]["MaxAmmo"];
    } else {
        return -1;
    }
}
AmmoPickupSystem::PlayerClass AmmoPickupSystem::DetermineClass(EntityWrapper &player)
{
    //determine the class based on what component the inflictor-player has
    if (m_World->HasComponent(player.ID, "DashAbility")) {
        return PlayerClass::Assault;
    }
    if (m_World->HasComponent(player.ID, "ShieldAbility")) {
        return PlayerClass::Defender;
    }
    if (m_World->HasComponent(player.ID, "SprintAbility")) {
        return PlayerClass::Sniper;
    }
    return PlayerClass::None;
}

bool AmmoPickupSystem::OnTriggerTouch(Events::TriggerTouch& e)
{
    if (!e.Entity.Valid()) {
        return false;
    }
    //TODO: add other weapontypes
    if (DetermineClass(e.Entity) == PlayerClass::None) {
        return false;
    }
    if (!e.Trigger.HasComponent("AmmoPickup")) {
        return false;
    }

    //if at maxammo, save the trigger-touch to a vector since standing inside it will not re-trigger the trigger
    if (DoesPlayerHaveMaxAmmo(e.Entity)) {
        m_PickupAtMaximum.push_back({ e.Entity, e.Trigger });
        return false;
    }
    DoPickup(e.Entity, e.Trigger);
    return true;
}

bool AmmoPickupSystem::OnAmmoPickup(Events::AmmoPickup & e)
{
    if (!e.Player.Valid()) {
        return false;
    }
    //TODO: add other weapontypes
    if (DetermineClass(e.Player) == PlayerClass::None) {
        return false;
    }
    //cant pick up ammopacks if you are already at MaxAmmo
    if (DoesPlayerHaveMaxAmmo(e.Player)) {
        return false;
    }
    int maxWeaponAmmo = GetPlayerMaxAmmo(e.Player);
    int& currentAmmo = GetPlayerAmmo(e.Player);

    currentAmmo = std::min(currentAmmo + e.AmmoGain, maxWeaponAmmo);

    return false;
}

bool AmmoPickupSystem::OnTriggerLeave(Events::TriggerLeave& e) {
    if (!e.Trigger.HasComponent("AmmoPickup")) {
        return false;
    }
    //triggerleave erases possible m_PickupAtMaximum
    for (auto& it = m_PickupAtMaximum.begin(); it != m_PickupAtMaximum.end(); ++it) {
        if (it->trigger.ID == e.Trigger.ID && it->player.ID == e.Entity.ID) {
            m_PickupAtMaximum.erase(it);
            break;
        }
    }
    return true;
}

void AmmoPickupSystem::DoPickup(EntityWrapper &player, EntityWrapper &trigger) {
    //trigger should be valid but if it isnt we just return (to avoid crash)
    if (!trigger.Valid()) {
        return;
    }
    int maxWeaponAmmo = GetPlayerMaxAmmo(player);
    int ammoGiven = 0.01*(double)trigger["AmmoPickup"]["AmmoGain"] * maxWeaponAmmo;

    Events::AmmoPickup ePlayerAmmoPickup;
    ePlayerAmmoPickup.AmmoGain = ammoGiven;
    ePlayerAmmoPickup.Player = player;
    m_EventBroker->Publish(ePlayerAmmoPickup);

    //copy position, ammogain, respawntimer (twice since one of the values will be counted down to 0, the other will be set in the new object)
    //we need to copy all values since each value can be different for each ammoPickup
    m_ETriggerTouchVector.push_back({ (glm::vec3)trigger["Transform"]["Position"], trigger["AmmoPickup"]["AmmoGain"],
        trigger["AmmoPickup"]["RespawnTimer"], trigger["AmmoPickup"]["RespawnTimer"], m_World->GetParent(trigger.ID) });

    //delete the ammopickup
    m_World->DeleteEntity(trigger.ID);
}
