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
                EntityFileParser parser(entityFile);
                EntityID ammoPickupID = parser.MergeEntities(m_World);

                //let the world know a pickup has spawned
                Events::PickupSpawned ePickupSpawned;
                ePickupSpawned.Pickup = EntityWrapper(m_World, ammoPickupID);
                m_EventBroker->Publish(ePickupSpawned);

                //copy values from the old entity to the new entity
                auto& newAmmoPickupEntity = EntityWrapper(m_World, ammoPickupID);
                newAmmoPickupEntity["Transform"]["Position"] = somePickup.Pos;
                newAmmoPickupEntity["AmmoPickup"]["AmmoGain"] = somePickup.AmmoGain;
                newAmmoPickupEntity["AmmoPickup"]["RespawnTimer"] = somePickup.RespawnTimer;
                m_World->SetParent(newAmmoPickupEntity.ID, somePickup.parentID);

                //erase the current element (somePickup)
                it = m_ETriggerTouchVector.erase(it);
            }
            else { 
                it++;
            }
        }
        //still touching m_PickupAtMaximum?
        for (auto& it = m_PickupAtMaximum.begin(); it != m_PickupAtMaximum.end(); ++it) {
            if (!it->player.Valid()) {
                m_PickupAtMaximum.erase(it);
                break;
            }
            if ((int)it->player["AssaultWeapon"]["Ammo"] < (int)it->player["AssaultWeapon"]["MaxAmmo"]) {
                DoPickup(it->player, it->trigger);
                m_PickupAtMaximum.erase(it);
                break;
            }
        }
    }
}

bool AmmoPickupSystem::OnTriggerTouch(Events::TriggerTouch& e)
{
    if (!e.Entity.Valid()) {
        return false;
    }
    //TODO: add other weapontypes
    if (!e.Entity.HasComponent("AssaultWeapon")) {
        return false;
    }
    if (!e.Trigger.HasComponent("AmmoPickup")) {
        return false;
    }

    //if at maxammo, save the trigger-touch to a vector since standing inside it will not re-trigger the trigger
    if ((int)e.Entity["AssaultWeapon"]["Ammo"] >= (int)e.Entity["AssaultWeapon"]["MaxAmmo"]) {
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
    if (!e.Player.HasComponent("AssaultWeapon")) {
        return false;
    }
    int maxWeaponAmmo = (int)e.Player["AssaultWeapon"]["MaxAmmo"];
    int& currentAmmo = (int)e.Player["AssaultWeapon"]["Ammo"];
    //cant pick up ammopacks if you are already at MaxAmmo
    if (currentAmmo >= maxWeaponAmmo) {
        return false;
    }

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
    int maxWeaponAmmo = (int)player["AssaultWeapon"]["MaxAmmo"];
    int& currentAmmo = (int)player["AssaultWeapon"]["Ammo"];
    int ammoGiven = 0.01*(double)trigger["AmmoPickup"]["AmmoGain"] * maxWeaponAmmo;

    Events::AmmoPickup ePlayerAmmoPickup;
    ePlayerAmmoPickup.AmmoGain = ammoGiven;
    ePlayerAmmoPickup.Player = player;
    m_EventBroker->Publish(ePlayerAmmoPickup);

    //immediately give the player the ammo (on server)
    currentAmmo = std::min(currentAmmo + ammoGiven, maxWeaponAmmo);

    //copy position, ammogain, respawntimer (twice since one of the values will be counted down to 0, the other will be set in the new object)
    //we need to copy all values since each value can be different for each ammoPickup
    m_ETriggerTouchVector.push_back({ (glm::vec3)trigger["Transform"]["Position"], trigger["AmmoPickup"]["AmmoGain"],
        trigger["AmmoPickup"]["RespawnTimer"], trigger["AmmoPickup"]["RespawnTimer"], m_World->GetParent(trigger.ID) });

    //delete the ammopickup
    m_World->DeleteEntity(trigger.ID);
}
