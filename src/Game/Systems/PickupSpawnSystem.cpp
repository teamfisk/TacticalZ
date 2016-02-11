#include "Systems/PickupSpawnSystem.h"

PickupSpawnSystem::PickupSpawnSystem(SystemParams params)
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_ETriggerTouch, &PickupSpawnSystem::OnTriggerTouch);
}

void PickupSpawnSystem::Update(double dt)
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

            //erase the current element (healthPickupPosition)
            m_ETriggerTouchVector.erase(it);
            break;
        }
    }
}


bool PickupSpawnSystem::OnTriggerTouch(Events::TriggerTouch& e)
{
    if (!e.Trigger.HasComponent("HealthPickup")) {
        return false;
    }
    double healthGiven = 0.01*(double)e.Trigger["HealthPickup"]["HealthGain"] * (double)e.Entity["Health"]["MaxHealth"];
    //cant pick up healthpacks if you are already at MaxHealth
    if ((double)e.Entity["Health"]["Health"] >= (double)e.Entity["Health"]["MaxHealth"]) {
        return false;
    }

    //personEntered = e.Entity, thingEntered = e.Trigger
    Events::PlayerHealthPickup ePlayerHealthPickup;
    ePlayerHealthPickup.HealthAmount = healthGiven;
    ePlayerHealthPickup.Player = e.Entity;
    m_EventBroker->Publish(ePlayerHealthPickup);

    //copy position, healthgain, respawntimer (twice since one of the values will be counted down to 0, the other will be set in the new object)
    //we need to copy all values since each value can be different for each healthPickup
    m_ETriggerTouchVector.push_back({ (glm::vec3)e.Trigger["Transform"]["Position"] ,e.Trigger["HealthPickup"]["HealthGain"],
        e.Trigger["HealthPickup"]["RespawnTimer"],e.Trigger["HealthPickup"]["RespawnTimer"] });

    //delete the healthpickup
    m_World->DeleteEntity(e.Trigger.ID);
    return true;
}
