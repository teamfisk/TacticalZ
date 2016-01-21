#include "Systems/WeaponSystem.h"

WeaponSystem::WeaponSystem(EventBroker* eventBroker, IRenderer* renderer)
    : System(eventBroker)
    , ImpureSystem()
    , m_Renderer(renderer)
{
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &WeaponSystem::OnInputCommand);
    EVENT_SUBSCRIBE_MEMBER(m_EShoot, &WeaponSystem::OnShoot);
}

void WeaponSystem::Update(World* world, double dt)
{
    for (int i = m_EShootVector.size(); i > 0; i--)
    {
        //TODO: check if player has enough ammo and if weapon has a cooldown or not

        //pick the object
        PickData pickDataFromShot = m_Renderer->Pick(std::get<1>(m_EShootVector[i - 1]));
        if (pickDataFromShot.Entity == EntityID_Invalid) {
            m_EShootVector.erase(m_EShootVector.begin() + i - 1);
            continue;
        }
        //if its a player, do PlayerDamage event
        const bool hasPlayerComponent = world->HasComponent(pickDataFromShot.Entity, "Player");
        if (hasPlayerComponent) {
            Events::PlayerDamage ePlayerDamage;
            //TODO: damage based on weapontype/class?
            //TODO: multiple shots at the same time? (shotgunner)
            ePlayerDamage.DamageAmount = 25;
            ePlayerDamage.PlayerDamagedID = pickDataFromShot.Entity;
            m_EventBroker->Publish(ePlayerDamage);
        }
        m_EShootVector.erase(m_EShootVector.begin() + i - 1);
    }
}

bool WeaponSystem::OnInputCommand(const Events::InputCommand& e)
{
    if (e.Command == "PrimaryFire" && e.Value > 0) {
        Events::Shoot eShoot;
        eShoot.shooter = e.PlayerID;
        m_EventBroker->Publish(eShoot);
    }
    return true;
}
bool WeaponSystem::OnShoot(const Events::Shoot& e) {
    //screen center, based on current resolution!
    Rectangle screenResolution = m_Renderer->Resolution();
    glm::vec2 centerScreen = glm::vec2(screenResolution.Width / 2, screenResolution.Height / 2);
    m_EShootVector.push_back(std::make_pair(e.shooter, centerScreen));
    return true;
}