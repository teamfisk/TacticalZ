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
    //LOAD THE XML
    auto deathEffect = ResourceManager::Load<EntityFile>("Schema/Entities/PlayerDeathExplosionWithCamera.xml");

    EntityFileParser parser(deathEffect);
    EntityID deathEffectID = parser.MergeEntities(m_World);
    EntityWrapper deathEffectEW = EntityWrapper(m_World, deathEffectID);
    
    //current components for player that we need
    auto playerModelEW = e.Player.FirstChildByName("PlayerModel");
    auto playerEntityModel = playerModelEW["Model"];
    auto playerEntityTransform = playerModelEW["Transform"];

    //copy the data from player to new playermodel
    playerEntityModel.Copy(deathEffectEW["Model"]);
    playerEntityTransform.Copy(deathEffectEW["Transform"]);

    //change the animation speed and make sure the explosioneffect spawns at the players position
    //http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
    auto playerPosition = (glm::vec3)e.Player["Transform"]["Position"];
    deathEffectEW["Transform"]["Position"] = playerPosition;
    deathEffectEW["ExplosionEffect"]["ExplosionOrigin"] = playerPosition;

    //camera (with lifetime) slightly above the player and looking down at the player
    //camera will be positioned just above the player
    glm::vec3 cameraPosition = glm::vec3(0, 10, 0);
    auto cam = deathEffectEW.FirstChildByName("Camera");
    cam["Transform"]["Position"] = cameraPosition;
    Events::SetCamera eSetCamera;
    eSetCamera.CameraEntity = cam;
    m_EventBroker->Publish(eSetCamera);

    //on deathanim done -> del entity
    m_World->DeleteEntity(e.Player.ID);
    return true;
}
