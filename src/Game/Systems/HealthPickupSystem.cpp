#include "Systems/HealthPickupSystem.h"

HealthPickupSystem::HealthPickupSystem(SystemParams params)
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_ETriggerTouch, &HealthPickupSystem::OnTriggerTouch);
    EVENT_SUBSCRIBE_MEMBER(m_ETriggerLeave, &HealthPickupSystem::OnTriggerLeave);
}

void HealthPickupSystem::Update(double dt)
{
    for (auto it = m_ETriggerTouchVector.begin(); it != m_ETriggerTouchVector.end(); ++it)
    {
        auto& healthPickupPosition = *it;
        //set the double timer value (value 3)
        healthPickupPosition.DecreaseThisRespawnTimer -= dt;
        if (healthPickupPosition.DecreaseThisRespawnTimer < 0) {
            //spawn and delete the vector item
            auto entityFile = ResourceManager::Load<EntityFile>("Schema/Entities/HealthPickup.xml");
            EntityFileParser parser(entityFile);
            EntityID healthPickupID = parser.MergeEntities(m_World);

            //let the world know a pickup has spawned (graphics effects, etc)
            Events::PickupSpawned ePickupSpawned;
            ePickupSpawned.Pickup = EntityWrapper(m_World, healthPickupID);
            m_EventBroker->Publish(ePickupSpawned);

            //set values from the old entity to the new entity
            auto& newHealthPickupEntity = EntityWrapper(m_World, healthPickupID);
            newHealthPickupEntity["Transform"]["Position"] = healthPickupPosition.Pos;
            newHealthPickupEntity["HealthPickup"]["HealthGain"] = healthPickupPosition.HealthGain;
            newHealthPickupEntity["HealthPickup"]["RespawnTimer"] = healthPickupPosition.RespawnTimer;
            m_World->SetParent(newHealthPickupEntity.ID, healthPickupPosition.parentID);

            //erase the current element (healthPickupPosition)
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
        if ((double)it->player["Health"]["Health"] < (double)it->player["Health"]["MaxHealth"]) {
            DoPickup(it->player, it->pickup);
            m_PickupAtMaximum.erase(it);
            break;

        }
    }
}

bool HealthPickupSystem::OnTriggerTouch(Events::TriggerTouch& e)
{
    if (!e.Trigger.HasComponent("HealthPickup")) {
        return false;
    }
    //cant pick up healthpacks if you are already at MaxHealth
    if ((double)e.Entity["Health"]["Health"] >= (double)e.Entity["Health"]["MaxHealth"]) {
        m_PickupAtMaximum.push_back({ e.Entity, e.Trigger });
        return false;
    }

    DoPickup(e.Entity, e.Trigger);
    return true;
}
bool HealthPickupSystem::OnTriggerLeave(Events::TriggerLeave& e) {
    //triggerleave erases possible m_PickupAtMaximum
    for (auto& it = m_PickupAtMaximum.begin(); it != m_PickupAtMaximum.end(); ++it) {
        if (it->pickup.ID == e.Trigger.ID && it->player.ID == e.Entity.ID) {
            m_PickupAtMaximum.erase(it);
            break;
        }
    }
    return true;
}

void HealthPickupSystem::DoPickup(EntityWrapper &player, EntityWrapper &trigger) {
    double healthGiven = 0.01*(double)trigger["HealthPickup"]["HealthGain"] * (double)player["Health"]["MaxHealth"];

    //personEntered = e.Entity, thingEntered = e.Trigger
    Events::PlayerHealthPickup ePlayerHealthPickup;
    ePlayerHealthPickup.HealthAmount = healthGiven;
    ePlayerHealthPickup.Player = player;
    m_EventBroker->Publish(ePlayerHealthPickup);

    //copy position, healthgain, respawntimer (twice since one of the values will be counted down to 0, the other will be set in the new object)
    //we need to copy all values since each value can be different for each healthPickup
    m_ETriggerTouchVector.push_back({ (glm::vec3)trigger["Transform"]["Position"] ,trigger["HealthPickup"]["HealthGain"],
        trigger["HealthPickup"]["RespawnTimer"],trigger["HealthPickup"]["RespawnTimer"], m_World->GetParent(trigger.ID) });

    //delete the healthpickup
    m_World->DeleteEntity(trigger.ID);
}

