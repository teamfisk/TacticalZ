#include "Systems/PlayerDeathSystem.h"

PlayerDeathSystem::PlayerDeathSystem(SystemParams params)
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_OnPlayerDeath, &PlayerDeathSystem::OnPlayerDeath);
    EVENT_SUBSCRIBE_MEMBER(m_EEntityDeleted, &PlayerDeathSystem::OnEntityDeleted);
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
    auto entityFile = ResourceManager::Load<EntityFile>("Schema/Entities/PlayerDeathExplosionWithCamera.xml");
    EntityWrapper deathEffectEW = entityFile->MergeInto(m_World);

    //components that we need from player
    auto playerModel = player.FirstChildByName("PlayerModel");
    if (!playerModel.Valid()) {
        return;
    }
    if (!playerModel.HasComponent("Model") || !playerModel.HasComponent("Animation")) {
        return;
    }
    auto playerEntityModel = playerModel["Model"];
    auto playerEntityAnimation = playerModel["Animation"];

    //copy the data from player to explosioneffectmodel
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
        m_LocalPlayerDeathEffect = deathEffectEW;
        auto cam = deathEffectEW.FirstChildByName("Camera");
        Events::SetCamera eSetCamera;
        eSetCamera.CameraEntity = cam;
        m_EventBroker->Publish(eSetCamera);
    }
}

bool PlayerDeathSystem::OnEntityDeleted(Events::EntityDeleted& e)
{
    // We only care about when the local players death effect is removed.
    if (m_LocalPlayerDeathEffect.ID != e.DeletedEntity) {
        return false;
    }
    
    // Look for the spectator camera entity in the level.
    EntityWrapper spectatorCam = m_World->GetFirstEntityByName("SpectatorCamera");
    if (!spectatorCam.Valid() || !spectatorCam.HasComponent("Camera") || LocalPlayer.Valid()) {
        return false;
    }
    Events::SetCamera eSetCamera;
    eSetCamera.CameraEntity = spectatorCam;
    m_EventBroker->Publish(eSetCamera);
    return true;
}
