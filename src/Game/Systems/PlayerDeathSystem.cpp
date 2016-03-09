#include "Systems/PlayerDeathSystem.h"
#include "Core/ELockMouse.h"

PlayerDeathSystem::PlayerDeathSystem(SystemParams params)
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_OnPlayerDeath, &PlayerDeathSystem::OnPlayerDeath);
    EVENT_SUBSCRIBE_MEMBER(m_EEntityDeleted, &PlayerDeathSystem::OnEntityDeleted);
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &PlayerDeathSystem::OnInputCommand);
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
    if (!playerModel.HasComponent("Model") || !playerModel.HasComponent("Animation")) {
        if (player == LocalPlayer) {
            setSpectatorCamera();
        }
        return;
    }
    auto playerEntityModel = playerModel["Model"];
    auto playerEntityAnimation = playerModel["Animation"];

    //copy the data from player to explosioneffectmodel
    playerEntityModel.Copy(deathEffectEW["Model"]);
    playerEntityAnimation.Copy(deathEffectEW["Animation"]);

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

    // If the player hasn't spawned already, activate the spectator camera.
    if (!LocalPlayer.Valid()) {
        setSpectatorCamera();
    }
    return true;
}

void PlayerDeathSystem::setSpectatorCamera()
{
    // Look for the spectator camera entity in the level.
    EntityWrapper spectatorCam = m_World->GetFirstEntityByName("SpectatorCamera");
    if (!spectatorCam.HasComponent("Camera")) {
        return;
    }
    Events::SetCamera eSetCamera;
    eSetCamera.CameraEntity = spectatorCam;
    m_EventBroker->Publish(eSetCamera);
    Events::UnlockMouse unlock;
    m_EventBroker->Publish(unlock);
}

bool PlayerDeathSystem::OnInputCommand(Events::InputCommand& e)
{
    if (e.Value == 0 ||  e.Command != "SwapToTeamPick" && e.Command != "SwapToClassPick") {
        return false;
    }

    // Ensure that we don't set spectator camera if the player deliberately changes to class/team pick.
    m_LocalPlayerDeathEffect = EntityWrapper::Invalid;
    return true;
}