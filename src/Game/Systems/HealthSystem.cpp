#include "Systems/HealthSystem.h"

HealthSystem::HealthSystem(SystemParams params)
    : System(params)
    , PureSystem("Health")
{
    //subscribe/listenTo playerdamage,healthpickup events (using the eventBroker)
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerDamage, &HealthSystem::OnPlayerDamaged);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerHealthPickup, &HealthSystem::OnPlayerHealthPickup);
    EVENT_SUBSCRIBE_MEMBER(m_InputCommand, &HealthSystem::OnInputCommand);
    m_NetworkEnabled = ResourceManager::Load<ConfigFile>("Config.ini")->Get("Networking.StartNetwork", false);
}

void HealthSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cHealth, double dt)
{
    double& health = cHealth["Health"];
    int healthInt = (int)health;
    //update struct
    for (auto& iter = m_DeadPlayers.begin(); iter < m_DeadPlayers.end(); iter++)
    {
        iter->timeSinceDeath += dt;
        if (iter->timeSinceDeath > 2.0f || !iter->playerEntity.Valid()) {
            m_DeadPlayers.erase(iter);
            break;
        }
    }
    if (healthInt <= 0) {
        for (auto deadPlayer : m_DeadPlayers)
        {
            if (deadPlayer.playerEntity.ID == entity.ID) {
                return;
            }
        }
        LOG_INFO("-> health <= 0");
        Events::PlayerDeath ePlayerDeath;
        ePlayerDeath.Player = entity;
        m_EventBroker->Publish(ePlayerDeath);
        m_DeadPlayers.emplace_back(entity);
        //Note: we will delete the entity in PlayerDeathSystem
    }
}

bool HealthSystem::OnPlayerDamaged(Events::PlayerDamage& e)
{
    LOG_INFO("<- on player damage");
    if (!IsServer && m_NetworkEnabled || !e.Victim.Valid()) {
        return false;
    }

    ComponentWrapper cHealth = e.Victim["Health"];
    double& health = cHealth["Health"];
    health -= e.Damage;
    LOG_INFO("-> health onplayerdam");

    return true;
}

bool HealthSystem::OnInputCommand(Events::InputCommand& e)
{
    if (e.Command == "TakeDamage" && e.Value > 0 && LocalPlayer.Valid()) {
        Events::PlayerDamage ev;
        ev.Inflictor = LocalPlayer;
        ev.Victim = LocalPlayer;
        ev.Damage = e.Value;
        m_EventBroker->Publish(ev);
    }
    return true;
}

bool HealthSystem::OnPlayerHealthPickup(Events::PlayerHealthPickup& e)
{
    ComponentWrapper cHealth = e.Player["Health"];
    double& health = cHealth["Health"];
    health += e.HealthAmount;
    health = std::min(health, (double)cHealth["MaxHealth"]);

    return true;
}

