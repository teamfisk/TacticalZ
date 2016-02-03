#include "Systems/PlayerDeathSystem.h"

PlayerDeathSystem::PlayerDeathSystem(World* m_World, EventBroker* eventBroker)
    : System(m_World, eventBroker)
{
    EVENT_SUBSCRIBE_MEMBER(m_OnInputCommand, &PlayerDeathSystem::OnInputCommand);
    EVENT_SUBSCRIBE_MEMBER(m_OnPlayerDeath, &PlayerDeathSystem::OnPlayerDeath);
}

bool PlayerDeathSystem::OnInputCommand(const Events::InputCommand& e)
{
    //testing: Jump > playerdamage
    if (e.Command != "Jump") {
        return false;

    }

    // 0 = released
    if (e.Value != 0) {
        return false;

    }

    auto players = m_World->GetComponents("Player");

    for (auto& cPlayer : *players) {
        EntityWrapper player(m_World, cPlayer.EntityID);
        Events::PlayerDamage e;
        e.Player = player;
        e.Damage = 50;
        m_EventBroker->Publish(e);

    }



    return true;
}

void PlayerDeathSystem::Update(double dt)
{
}

bool PlayerDeathSystem::OnPlayerDeath(Events::PlayerDeath& e)
{
    //load the explosioneffect XML
    auto deathEffect = ResourceManager::Load<EntityFile>("Schema/Entities/PlayerDeathExplosionWithCamera.xml");

    EntityFileParser parser(deathEffect);
    EntityID deathEffectID = parser.MergeEntities(m_World);
    EntityWrapper deathEffectEW = EntityWrapper(m_World, deathEffectID);

    //components that we need from player
    auto playerCamera = e.Player.FirstChildByName("Camera");
    auto playerEntityModel = e.Player.FirstChildByName("PlayerModel")["Model"];
    auto playerEntityAnimation = e.Player.FirstChildByName("PlayerModel")["Animation"];

    //copy the data from player to explisioneffectmodel
    playerEntityModel.Copy(deathEffectEW["Model"]);
    playerEntityAnimation.Copy(deathEffectEW["Animation"]);
    //freeze the animation
    deathEffectEW["Animation"]["Speed"] = 0.0;

    //copy the models position,orientation
    deathEffectEW["Transform"]["Position"] = (glm::vec3)e.Player["Transform"]["Position"];
    deathEffectEW["Transform"]["Orientation"] = (glm::vec3)e.Player["Transform"]["Orientation"];
    //effect,camera is relative to playersPosition
    deathEffectEW["ExplosionEffect"]["ExplosionOrigin"] = glm::vec3(0, 0, 0);

    //camera (with lifetime) behind the player
    auto cam = deathEffectEW.FirstChildByName("Camera");
    cam["Transform"]["Position"] = glm::vec3(0, 2.5f, 1.8f);
    cam["Transform"]["Orientation"] = glm::vec3(5.655f, 0, 0);
    Events::SetCamera eSetCamera;
    eSetCamera.CameraEntity = cam;
    m_EventBroker->Publish(eSetCamera);

    //done -> del entity
    m_World->DeleteEntity(e.Player.ID);
    return true;
}
