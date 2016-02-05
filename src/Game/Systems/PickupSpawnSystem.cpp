#include "Systems/PickupSpawnSystem.h"

PickupSpawnSystem::PickupSpawnSystem(World* m_World, EventBroker* eventBroker)
    : System(m_World, eventBroker)
{
    EVENT_SUBSCRIBE_MEMBER(m_ETriggerTouch, &PickupSpawnSystem::OnTriggerTouch);
}

void PickupSpawnSystem::Update(double dt)
{
    for (auto &healthPickupPosition : m_ETriggerTouchVector) {
        //set the double timer value (1)
        std::get<1>(healthPickupPosition) -= dt;
        if (std::get<1>(healthPickupPosition) < 0) {
            //spawn and delete the vector item
            auto entityFile = ResourceManager::Load<EntityFile>("Schema/Entities/HealthPickup.xml");
            EntityFileParser parser(entityFile);
            EntityID healthPickupID = parser.MergeEntities(m_World);

            //let the world know a pickup has spawned (graphics effects, etc)
            Events::PickupSpawned ePickupSpawned;
            ePickupSpawned.PickupID = healthPickupID;
            ePickupSpawned.Pickup = EntityWrapper(m_World, healthPickupID);
            m_EventBroker->Publish(ePickupSpawned);

            //erase the current element (healthPickupPosition)
            m_ETriggerTouchVector.erase(std::remove(m_ETriggerTouchVector.begin(), m_ETriggerTouchVector.end(),healthPickupPosition), m_ETriggerTouchVector.end());
            break;
        }
    }
}


bool PickupSpawnSystem::OnTriggerTouch(Events::TriggerTouch& e)
{
    if (!e.Trigger.HasComponent("HealthPickup")) {
        return false;
    }
    //personEntered = e.Entity, thingEntered = e.Trigger
    Events::PlayerHealthPickup ePlayerHealthPickup;
    ePlayerHealthPickup.HealthAmount = 30.0f;
    ePlayerHealthPickup.PlayerHealedID = e.Entity.ID;
    m_EventBroker->Publish(ePlayerHealthPickup);

    //copy the position to a vector -> respawntimer in ["HealthPickup"]["RespawnTimer"]
    m_ETriggerTouchVector.push_back(std::make_tuple((glm::vec3)e.Trigger["Transform"]["Position"], e.Trigger["HealthPickup"]["RespawnTimer"]));

    //delete the healthpickup
    m_World->DeleteEntity(e.Trigger.ID);
    return true;
}
