#include "Systems/AmmoPickupSystem.h"

AmmoPickupSystem::AmmoPickupSystem(SystemParams params)
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_ETriggerTouch, &AmmoPickupSystem::OnTriggerTouch);
}

void AmmoPickupSystem::Update(double dt)
{
    for (auto it = m_ETriggerTouchVector.begin(); it != m_ETriggerTouchVector.end(); ++it)
    {
        auto& ammoPickupPosition = *it;
        //set the double timer value (value 3)
        ammoPickupPosition.DecreaseThisRespawnTimer -= dt;
        if (ammoPickupPosition.DecreaseThisRespawnTimer < 0.0) {
            //spawn and delete the vector item
            auto entityFile = ResourceManager::Load<EntityFile>("Schema/Entities/AmmoPickup.xml");
            EntityFileParser parser(entityFile);
            EntityID ammoPickupID = parser.MergeEntities(m_World);

            //let the world know a pickup has spawned (graphics effects, etc)
            Events::PickupSpawned ePickupSpawned;
            ePickupSpawned.Pickup = EntityWrapper(m_World, ammoPickupID);
            m_EventBroker->Publish(ePickupSpawned);

            //set values from the old entity to the new entity
            auto& newAmmoPickupEntity = EntityWrapper(m_World, ammoPickupID);
            newAmmoPickupEntity["Transform"]["Position"] = ammoPickupPosition.Pos;
            newAmmoPickupEntity["AmmoPickup"]["AmmoGain"] = ammoPickupPosition.AmmoGain;
            newAmmoPickupEntity["AmmoPickup"]["RespawnTimer"] = ammoPickupPosition.RespawnTimer;

            //erase the current element (AmmoPickupPosition)
            m_ETriggerTouchVector.erase(it);
            break;
        }
    }
}


bool AmmoPickupSystem::OnTriggerTouch(Events::TriggerTouch& e)
{
    if (e.Entity != LocalPlayer) {
        return false;
    }
    //TODO: add other weapontypes
    if (!e.Entity.HasComponent("AssaultWeapon")) {
        return false;
    }
    if (!e.Trigger.HasComponent("AmmoPickup")) {
        return false;
    }
    int maxWeaponAmmo = (int)e.Entity["AssaultWeapon"]["MaxAmmo"];
    int& currentAmmo = (int)e.Entity["AssaultWeapon"]["Ammo"];

    int ammoGiven = 0.01*(double)e.Trigger["AmmoPickup"]["AmmoGain"] * maxWeaponAmmo;
    //cant pick up ammopacks if you are already at MaxAmmo
    if (currentAmmo >= maxWeaponAmmo) {
        return false;
    }

    //personEntered = e.Entity, thingEntered = e.Trigger
    Events::AmmoPickup ePlayerAmmoPickup;
    ePlayerAmmoPickup.AmmoGain = ammoGiven;
    ePlayerAmmoPickup.Player = e.Entity;
    m_EventBroker->Publish(ePlayerAmmoPickup);
    //immediately give the player the ammo
    currentAmmo = std::min(currentAmmo + ammoGiven, maxWeaponAmmo);

    //copy position, ammogain, respawntimer (twice since one of the values will be counted down to 0, the other will be set in the new object)
    //we need to copy all values since each value can be different for each ammoPickup
    m_ETriggerTouchVector.push_back({ (glm::vec3)e.Trigger["Transform"]["Position"] ,e.Trigger["AmmoPickup"]["AmmoGain"],
        e.Trigger["AmmoPickup"]["RespawnTimer"],e.Trigger["AmmoPickup"]["RespawnTimer"] });

    //delete the ammopickup
    m_World->DeleteEntity(e.Trigger.ID);
    return true;
}
