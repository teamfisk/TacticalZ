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
{ }

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

    auto playerModel = player.FirstChildByName("PlayerModel");
    if (!playerModel.Valid()) {
        if (player == LocalPlayer) {
            setSpectatorCamera();
        }
        return;
    }
    if (!playerModel.HasComponent("Model")) {
        return;
    }
    EntityWrapper deathEffect = playerModel.Clone();
    for (auto& cAnim : deathEffect.ChildrenWithComponent("Animation")) {
        cAnim["Animation"]["Play"] = false;
    }
    deathEffect.AttachComponent("ExplosionEffect");
    deathEffectEW["ExplosionEffect"].Copy(deathEffect["ExplosionEffect"]);
    deathEffect.AttachComponent("Lifetime");
    deathEffectEW["Lifetime"].Copy(deathEffect["Lifetime"]);
    deathEffect["Transform"]["Position"] = (glm::vec3)player["Transform"]["Position"];
    deathEffect["Transform"]["Orientation"] = (glm::vec3)player["Transform"]["Orientation"];
    for (auto& cModel : deathEffect.ChildrenWithComponent("Model")) {
        EntityWrapper e(m_World, cModel.ID);
        e.AttachComponent("ExplosionEffect");
        deathEffectEW["ExplosionEffect"].Copy(e["ExplosionEffect"]);
    }
    EntityWrapper cam = deathEffectEW.FirstChildByName("Camera").Clone(deathEffect);
    //camera (with lifetime) behind the player
    if (player == LocalPlayer) {
        m_LocalPlayerDeathEffect = deathEffect;
        Events::SetCamera eSetCamera;
        eSetCamera.CameraEntity = cam;
        m_EventBroker->Publish(eSetCamera);
    }
}

bool PlayerDeathSystem::OnEntityDeleted(Events::EntityDeleted& e)
{
    // We only care about when the local players death effect is removed.
    if (m_LocalPlayerDeathEffect != e.DeletedEntity) {
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