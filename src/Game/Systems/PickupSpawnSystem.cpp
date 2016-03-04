#include "Systems/PickupSpawnSystem.h"

PickupSpawnSystem::PickupSpawnSystem(SystemParams params)
    : System(params)
{
    if (IsServer) {
        EVENT_SUBSCRIBE_MEMBER(m_ETriggerTouch, &PickupSpawnSystem::OnTriggerTouch);
        EVENT_SUBSCRIBE_MEMBER(m_ETriggerLeave, &PickupSpawnSystem::OnTriggerLeave);
    }
}

void PickupSpawnSystem::Update(double dt)
{
    if (IsServer) {
        auto it = m_ETriggerTouchVector.begin();
        while (it != m_ETriggerTouchVector.end()) {
            auto& somePickup = *it;
            somePickup.DecreaseThisRespawnTimer -= dt;
            if (somePickup.DecreaseThisRespawnTimer < 0.0) {
                //spawn the new healthPickup
                auto entityFile = ResourceManager::Load<EntityFile>("Schema/Entities/HealthPickup.xml");
                EntityWrapper healthPickup = entityFile->MergeInto(m_World);

                //let the world know a pickup has spawned (graphics effects, etc)
                Events::PickupSpawned ePickupSpawned;
                ePickupSpawned.Pickup = healthPickup;
                m_EventBroker->Publish(ePickupSpawned);

                //copy values from the old entity to the new entity
                auto& newHealthPickupEntity = healthPickup;
                newHealthPickupEntity["Transform"]["Position"] = somePickup.Pos;
                newHealthPickupEntity["HealthPickup"]["HealthGain"] = somePickup.HealthGain;
                newHealthPickupEntity["HealthPickup"]["RespawnTimer"] = somePickup.RespawnTimer;
                m_World->SetParent(newHealthPickupEntity.ID, somePickup.parentID);

                //erase the current element (somePickup)
                it = m_ETriggerTouchVector.erase(it);
            } else {
                it++;
            }
        }
        //still touching PickupAtMaximum?
        for (auto& it = m_PickupAtMaximum.begin(); it != m_PickupAtMaximum.end(); ++it) {
            if (!it->player.Valid()) {
                m_PickupAtMaximum.erase(it);
                break;
            }
            if ((double)it->player["Health"]["Health"] < (double)it->player["Health"]["MaxHealth"]) {
                DoPickup(it->player, it->trigger);
                m_PickupAtMaximum.erase(it);
                break;
            }
        }
    }
}
bool PickupSpawnSystem::OnTriggerTouch(Events::TriggerTouch& e)
{
    if (!e.Trigger.HasComponent("HealthPickup")) {
        return false;
    }
    //if at maxhealth, save the trigger-touch to a vector since standing inside it will not re-trigger the trigger
    if ((double)e.Entity["Health"]["Health"] >= (double)e.Entity["Health"]["MaxHealth"]) {
        m_PickupAtMaximum.push_back({ e.Entity, e.Trigger });
        return false;
    }

    DoPickup(e.Entity, e.Trigger);
    return true;
}
bool PickupSpawnSystem::OnTriggerLeave(Events::TriggerLeave& e)
{
    if (!e.Trigger.HasComponent("HealthPickup")) {
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
void PickupSpawnSystem::DoPickup(EntityWrapper &player, EntityWrapper &trigger)
{
    double healthGiven = 0.01*(double)trigger["HealthPickup"]["HealthGain"] * (double)player["Health"]["MaxHealth"];

    //only the server will increase the players hp and set it in the next delta
    Events::PlayerHealthPickup ePlayerHealthPickup;
    ePlayerHealthPickup.HealthAmount = healthGiven;
    ePlayerHealthPickup.Player = player;
    m_EventBroker->Publish(ePlayerHealthPickup);

    //copy position, healthgain, respawntimer (twice since one of the values will be counted down to 0, the other will be set in the new object)
    //we need to copy all values since each value can be different for each healthPickup
    m_ETriggerTouchVector.push_back({ (glm::vec3)trigger["Transform"]["Position"], trigger["HealthPickup"]["HealthGain"],
        trigger["HealthPickup"]["RespawnTimer"], trigger["HealthPickup"]["RespawnTimer"], m_World->GetParent(trigger.ID) });

    //delete the healthpickup
    m_World->DeleteEntity(trigger.ID);
}
