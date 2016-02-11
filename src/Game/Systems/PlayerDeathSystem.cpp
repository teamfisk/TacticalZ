#include "Systems/PlayerDeathSystem.h"

PlayerDeathSystem::PlayerDeathSystem(SystemParams params)
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_OnPlayerDeath, &PlayerDeathSystem::OnPlayerDeath);
}

void PlayerDeathSystem::Update(double dt)
{
}

bool PlayerDeathSystem::OnPlayerDeath(Events::PlayerDeath& e)
{
    if (!e.Player.Valid()) {
        return false;
    }

    createDeathEffect(e.Player);

    // Delete player
    m_World->DeleteEntity(e.Player.ID);

    return true;
}

void PlayerDeathSystem::createDeathEffect(EntityWrapper player)
{
    //load the explosioneffect XML
    auto deathEffect = ResourceManager::Load<EntityFile>("Schema/Entities/PlayerDeathExplosionWithCamera.xml");
    EntityFileParser parser(deathEffect);
    EntityID deathEffectID = parser.MergeEntities(m_World);
    EntityWrapper deathEffectEW = EntityWrapper(m_World, deathEffectID);

    //components that we need from player
    auto playerCamera = player.FirstChildByName("Camera");
    auto playerEntityModel = player.FirstChildByName("PlayerModel")["Model"];
    auto playerEntityAnimation = player.FirstChildByName("PlayerModel")["Animation"];

    //copy the data from player to explisioneffectmodel
    playerEntityModel.Copy(deathEffectEW["Model"]);
    playerEntityAnimation.Copy(deathEffectEW["Animation"]);
    //freeze the animation
    deathEffectEW["Animation"]["Speed1"] = 0.0;
    deathEffectEW["Animation"]["Speed2"] = 0.0;
    deathEffectEW["Animation"]["Speed3"] = 0.0;

    //copy the models position,orientation
    deathEffectEW["Transform"]["Position"] = (glm::vec3)player["Transform"]["Position"];
    deathEffectEW["Transform"]["Orientation"] = (glm::vec3)player["Transform"]["Orientation"];
    //effect,camera is relative to playersPosition
    //deathEffectEW["ExplosionEffect"]["ExplosionOrigin"] = glm::vec3(0, 0, 0);

    //camera (with lifetime) behind the player
    if (player == LocalPlayer) {
        auto cam = deathEffectEW.FirstChildByName("Camera");
        Events::SetCamera eSetCamera;
        eSetCamera.CameraEntity = cam;
        m_EventBroker->Publish(eSetCamera);
    }
}
