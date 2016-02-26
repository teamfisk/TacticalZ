#include "Systems/AmmoPickupSystem.h"

AmmoPickupSystem::AmmoPickupSystem(SystemParams params)
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_ETriggerTouch, &AmmoPickupSystem::OnTriggerTouch);
    EVENT_SUBSCRIBE_MEMBER(m_ETriggerLeave, &AmmoPickupSystem::OnTriggerLeave);
}

void AmmoPickupSystem::Update(double dt)
{
    for (auto it = m_ETriggerTouchVector.begin(); it != m_ETriggerTouchVector.end(); ++it)
    {
        auto& somePickup = *it;

        //set the double timer value (value 3)
        somePickup.DecreaseThisRespawnTimer -= dt;
        if (somePickup.DecreaseThisRespawnTimer < 0.0) {
            //spawn and delete the vector item
            auto entityFile = ResourceManager::Load<EntityFile>("Schema/Entities/" + somePickup.isAmmoPickup ? "AmmoPickup.xml" : "HealthPickup.xml");
            EntityFileParser parser(entityFile);
            EntityID ammoPickupID = parser.MergeEntities(m_World);

            //let the world know a pickup has spawned (graphics effects, etc)
            Events::PickupSpawned ePickupSpawned;
            ePickupSpawned.Pickup = EntityWrapper(m_World, ammoPickupID);
            m_EventBroker->Publish(ePickupSpawned);

            //set values from the old entity to the new entity
            auto& newPickupEntity = EntityWrapper(m_World, ammoPickupID);
            newPickupEntity["Transform"]["Position"] = somePickup.Pos;
            if (somePickup.isAmmoPickup) {
                newPickupEntity["AmmoPickup"]["AmmoGain"] = somePickup.AmmoGain;
                newPickupEntity["AmmoPickup"]["RespawnTimer"] = somePickup.RespawnTimer;
            } else {
                newPickupEntity["HealthPickup"]["HealthGain"] = somePickup.AmmoGain;
                newPickupEntity["HealthPickup"]["RespawnTimer"] = somePickup.RespawnTimer;
            }
            m_World->SetParent(newPickupEntity.ID, somePickup.parentID);

            //erase the current element (somePickup)
            m_ETriggerTouchVector.erase(it);
            break;
        }
    }
    //still touching AmmoPickupAtMaxHealthAmmo?
    for (auto& it = m_PickupAtMaximum.begin(); it != m_PickupAtMaximum.end(); ++it) {
        if (!it->player.Valid() || !it->pickup.Valid()) {
            m_PickupAtMaximum.erase(it);
            break;
        }
        if ((int)it->player["AssaultWeapon"]["Ammo"] < (int)it->player["AssaultWeapon"]["MaxAmmo"]) {
            DoPickup(it->player, it->pickup);
            m_PickupAtMaximum.erase(it);
            break;
        }
    }
}


bool AmmoPickupSystem::OnTriggerTouch(Events::TriggerTouch& e)
{
    if (e.Entity != LocalPlayer) {
        return false;
    }
    if (!e.Trigger.HasComponent("AmmoPickup") && !e.Trigger.HasComponent("HealthPickup")) {
        return false;
    }
    //TODO: add other weapontypes
    if (!e.Entity.HasComponent("AssaultWeapon")) {
        return false;
    }
    int maxWeaponAmmo = (int)e.Entity["AssaultWeapon"]["MaxAmmo"];
    int& currentAmmo = (int)e.Entity["AssaultWeapon"]["Ammo"];
    //cant pick up ammopacks if you are already at MaxAmmo
    if (currentAmmo >= maxWeaponAmmo) {
        m_PickupAtMaximum.push_back({ e.Entity, e.Trigger });
        return false;
    }
    DoPickup(e.Entity, e.Trigger);
    return true;
}

bool AmmoPickupSystem::OnTriggerLeave(Events::TriggerLeave& e) {
    //triggerleave erases possible AmmoPickupAtMaxHealthAmmo
    for (auto& it = m_PickupAtMaximum.begin(); it != m_PickupAtMaximum.end(); ++it) {
        if (it->pickup.ID == e.Trigger.ID && it->player.ID == e.Entity.ID) {
            m_PickupAtMaximum.erase(it);
            break;
        }
    }
    return true;
}
void AmmoPickupSystem::DoPickup(EntityWrapper &player, EntityWrapper &trigger) {
    auto& weaponEntity = player["AssaultWeapon"];
    auto& pickup = trigger["AmmoPickup"];
    auto& gain = pickup["AmmoGain"];
    int maxValue = (int)weaponEntity["MaxAmmo"];
    int& currentValue = (int)weaponEntity["Ammo"];

    //cant pick up if you are already at max
    if (currentValue >= maxValue) {
        return;
    }

    int increaseGiven = 0.01*(double)gain * maxValue;
    Events::AmmoPickup ePlayerAmmoPickup;
    ePlayerAmmoPickup.AmmoGain = increaseGiven;
    ePlayerAmmoPickup.Player = player;
    m_EventBroker->Publish(ePlayerAmmoPickup);
    //immediately give the player the ammo
    currentValue = std::min(currentValue + increaseGiven, maxValue);

    //copy position, ammogain, respawntimer (twice since one of the values will be counted down to 0, the other will be set in the new object)
    //we need to copy all values since each value can be different for each ammoPickup
    m_ETriggerTouchVector.push_back({ trigger["Transform"]["Position"], gain,
        pickup["RespawnTimer"], pickup["RespawnTimer"], m_World->GetParent(trigger.ID) , true });

    //delete the ammopickup
    m_World->DeleteEntity(trigger.ID);
}
