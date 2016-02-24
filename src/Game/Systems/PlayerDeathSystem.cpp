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
    LOG_INFO("<- deathsystem: valid player");

    if (!e.Player.Valid()) {
        return false;
    }
    LOG_INFO("-> deathsystem: valid player");

    LOG_INFO("<- create effect");

    createDeathEffect(e.Player);

    LOG_INFO("-> create effect");

    if (!IsClient || !ResourceManager::Load<ConfigFile>("Config.ini")->Get("Networking.StartNetwork", false)) {
        m_World->DeleteEntity(e.Player.ID);
    }

    return true;
}

void PlayerDeathSystem::createDeathEffect(EntityWrapper player)
{
    //load the explosioneffect XML
    auto deathEffect = ResourceManager::Load<EntityFile>("Schema/Entities/PlayerDeathExplosionWithCamera.xml");
    EntityFileParser parser(deathEffect);
    EntityID deathEffectID = parser.MergeEntities(m_World);
    EntityWrapper deathEffectEW = EntityWrapper(m_World, deathEffectID);

    LOG_INFO("<- effect camera");

    //components that we need from player
    auto playerCamera = player.FirstChildByName("Camera");
    auto playerModel = player.FirstChildByName("PlayerModel");
    if (!playerCamera.Valid() || !playerModel.Valid()) {
        return;
    }
    if (!playerModel.HasComponent("Model") || !playerModel.HasComponent("Animation")) {
        return;
    }
    auto playerEntityModel = playerModel["Model"];
    auto playerEntityAnimation = playerModel["Animation"];
    LOG_INFO("-> effect camera");

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

    LOG_INFO("<- localplayer");
    //camera (with lifetime) behind the player
    if (player == LocalPlayer) {
        auto cam = deathEffectEW.FirstChildByName("Camera");
        Events::SetCamera eSetCamera;
        eSetCamera.CameraEntity = cam;
        m_EventBroker->Publish(eSetCamera);
        LOG_INFO("in localplayer");
    }
    LOG_INFO("-> localplayer");

}
