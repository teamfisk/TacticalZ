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
        PickData somePickData = m_Renderer->Pick(std::get<1>(m_EShootVector[i - 1]));
        if (somePickData.Entity == EntityID_Invalid) {
            m_EShootVector.erase(m_EShootVector.begin() + i - 1);
            continue;
        }
        //if its a player, do PlayerDamage event
        const bool hasPlayerComponent = world->HasComponent(somePickData.Entity, "Player");
        if (hasPlayerComponent) {
            Events::PlayerDamage ePlayerDamage;
            //TODO: damage based on weapontype/class?
            //TODO: multiple shots at the same time? (shotgunner)
            ePlayerDamage.DamageAmount = 25;
            ePlayerDamage.PlayerDamagedID = somePickData.Entity;
            ePlayerDamage.TypeOfDamage = "Some Weapon";
            m_EventBroker->Publish(ePlayerDamage);
            //tests:color
            m_TestDamageTotal += 0.25f;
            if (m_TestDamageTotal > 6.0f) {
                m_TestDamageTotal = 0.25f;
            }
            ComponentWrapper& playerModel = world->GetComponent(somePickData.Entity, "Model");
            playerModel["Color"] = glm::vec4(m_TestDamageTotal, 0, 0, 1);
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