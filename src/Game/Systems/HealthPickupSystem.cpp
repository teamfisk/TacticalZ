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
        auto& somePickup = *it;
        //set the double timer value (value 3)
        somePickup.DecreaseThisRespawnTimer -= dt;
        if (somePickup.DecreaseThisRespawnTimer < 0.0) {
            LOG_INFO("loading healthpickup xml");

            //spawn and delete the vector item
            auto entityFile = ResourceManager::Load<EntityFile>("Schema/Entities/HealthPickup.xml");
            EntityFileParser parser(entityFile);
            EntityID healthPickupID = parser.MergeEntities(m_World);
            LOG_INFO("finished loading healthpickup xml");

            //let the world know a pickup has spawned (graphics effects, etc)
            Events::PickupSpawned ePickupSpawned;
            ePickupSpawned.Pickup = EntityWrapper(m_World, healthPickupID);
            m_EventBroker->Publish(ePickupSpawned);
            LOG_INFO("event published");

            //set values from the old entity to the new entity
            auto& newHealthPickupEntity = EntityWrapper(m_World, healthPickupID);
            LOG_INFO("healthpickup id %i", healthPickupID);
            newHealthPickupEntity["Transform"]["Position"] = somePickup.Pos;
            newHealthPickupEntity["HealthPickup"]["HealthGain"] = somePickup.HealthGain;
            newHealthPickupEntity["HealthPickup"]["RespawnTimer"] = somePickup.RespawnTimer;
            m_World->SetParent(newHealthPickupEntity.ID, somePickup.parentID);
            LOG_INFO("healthpickup parented to %i", somePickup.parentID);
            //erase the current element (somePickup)
            m_ETriggerTouchVector.erase(it);
            break;
        }
    }
    //still touching AmmoPickupAtMaxHealthAmmo?
    for (auto& it = m_PickupAtMaximum.begin(); it != m_PickupAtMaximum.end(); ++it) {
        if (!it->player.Valid()) {
            LOG_INFO("player no longer valid, erasing maxPickup");
            m_PickupAtMaximum.erase(it);
            break;

        }
        if ((double)it->player["Health"]["Health"] < (double)it->player["Health"]["MaxHealth"]) {
            LOG_INFO("maxhealth found in pickupmax.. yada yada");
            DoPickup(it->player, it->pickup, it->trigger);
            m_PickupAtMaximum.erase(it);
            break;
        }
    }
}

bool HealthPickupSystem::OnTriggerTouch(Events::TriggerTouch& e)
{
    LOG_INFO("OnTriggerTouch");
    if (!e.Trigger.HasComponent("HealthPickup")) {
        return false;
    }
    LOG_INFO("touch start");

    //cant pick up healthpacks if you are already at MaxHealth
    if ((double)e.Entity["Health"]["Health"] >= (double)e.Entity["Health"]["MaxHealth"]) {
        LOG_INFO("triggertouch enter at maxhealth");
        //push back a copy of the entitys data, rather than e.trigger. since the server might delete e.trigger before client can use it
        m_PickupAtMaximum.push_back({ e.Entity,{ (glm::vec3)e.Trigger["Transform"]["Position"] ,e.Trigger["HealthPickup"]["HealthGain"],
            e.Trigger["HealthPickup"]["RespawnTimer"],e.Trigger["HealthPickup"]["RespawnTimer"], m_World->GetParent(e.Trigger.ID) } , e.Trigger });
        return false;
    }
    LOG_INFO("touch doing pickup");

    DoPickup(e.Entity, e.Trigger);
    return true;
}
bool HealthPickupSystem::OnTriggerLeave(Events::TriggerLeave& e) {
    //triggerleave erases possible m_PickupAtMaximum
    for (auto& it = m_PickupAtMaximum.begin(); it != m_PickupAtMaximum.end(); ++it) {
        if (it->trigger.ID == e.Trigger.ID && it->player.ID == e.Entity.ID) {
            LOG_INFO("trigger leave - erasing max");
            m_PickupAtMaximum.erase(it);
            break;
        }
    }
    return true;
}

void HealthPickupSystem::DoPickup(EntityWrapper &player, EntityWrapper &trigger) {
    double healthGiven = 0.01*(double)trigger["HealthPickup"]["HealthGain"] * (double)player["Health"]["MaxHealth"];
    LOG_INFO("do pickup start");

    //personEntered = e.Entity, thingEntered = e.Trigger
    //only the server will increase the players hp and set it in the next delta
    if (player.ID != LocalPlayer.ID && ResourceManager::Load<ConfigFile>("Config.ini")->Get("Networking.StartNetwork", false)) {
        Events::PlayerHealthPickup ePlayerHealthPickup;
        ePlayerHealthPickup.HealthAmount = healthGiven;
        ePlayerHealthPickup.Player = player;
        m_EventBroker->Publish(ePlayerHealthPickup);
        LOG_INFO("do pickup event published");
    }
    //copy position, healthgain, respawntimer (twice since one of the values will be counted down to 0, the other will be set in the new object)
    //we need to copy all values since each value can be different for each healthPickup
    m_ETriggerTouchVector.push_back({ (glm::vec3)trigger["Transform"]["Position"] ,trigger["HealthPickup"]["HealthGain"],
        trigger["HealthPickup"]["RespawnTimer"],trigger["HealthPickup"]["RespawnTimer"], m_World->GetParent(trigger.ID) });
    LOG_INFO("pickup set id to %i", trigger.ID);

    //delete the healthpickup
    m_World->DeleteEntity(trigger.ID);
}

//to be used with at-max-health-pickup
void HealthPickupSystem::DoPickup(EntityWrapper &player, NewHealthPickup &triggerCopy, EntityWrapper &trigger) {
    double healthGiven = 0.01*triggerCopy.HealthGain * (double)player["Health"]["MaxHealth"];
    LOG_INFO("do pickup start");

    //only the server will increase the players hp and set it in the next delta
    if (player.ID != LocalPlayer.ID && ResourceManager::Load<ConfigFile>("Config.ini")->Get("Networking.StartNetwork", false)) {
        Events::PlayerHealthPickup ePlayerHealthPickup;
        ePlayerHealthPickup.HealthAmount = healthGiven;
        ePlayerHealthPickup.Player = player;
        m_EventBroker->Publish(ePlayerHealthPickup);
        LOG_INFO("do pickup event published");
    }
    m_ETriggerTouchVector.push_back(triggerCopy);
    //LOG_INFO("pickup set id to %i", trigger.ID);

    //delete the healthpickup
    if (trigger.Valid()) {
        m_World->DeleteEntity(trigger.ID);
    }
}
