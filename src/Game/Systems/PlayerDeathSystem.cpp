#include "Systems/PlayerDeathSystem.h"

PlayerDeathSystem::PlayerDeathSystem(World* m_World, EventBroker* eventBroker)
    : System(m_World, eventBroker)
{
    EVENT_SUBSCRIBE_MEMBER(m_OnPlayerDeath, &PlayerDeathSystem::OnPlayerDeath);
}

void PlayerDeathSystem::Update(double dt)
{
}

bool PlayerDeathSystem::OnPlayerDeath(Events::PlayerDeath& e)
{
    //current components for player that we need
    auto playerModelEWrapper = e.Player.FirstChildByName("PlayerModel");
    auto playerEntityModel = playerModelEWrapper["Model"];
    auto playerEntityAnimation = playerModelEWrapper["Animation"];
    auto playerEntityTransform = playerModelEWrapper["Transform"];

    //create new entity with those components
    //graphics bug: model must have an animationcomponent to be able to display it
    auto newEntity = m_World->CreateEntity();
    ComponentWrapper& newEntityModel = m_World->AttachComponent(newEntity, "Model");
    ComponentWrapper& newAnimationModel = m_World->AttachComponent(newEntity, "Animation");
    ComponentWrapper& newTransformModel = m_World->AttachComponent(newEntity, "Transform");
    playerEntityModel.Copy(newEntityModel);
    playerEntityAnimation.Copy(newAnimationModel);
    playerEntityTransform.Copy(newTransformModel);

    //change the animation speed and make sure the explosioneffect spawns at the players position
    newAnimationModel["Speed"] = (double)0.0;
    newEntityModel["Color"] = glm::vec4(1, 0, 0, 1);
    newTransformModel["Position"] = (glm::vec3)e.Player["Transform"]["Position"];
    newTransformModel["Scale"] = glm::vec3(1, 1, 1);

    //add the explosion with a lifetime
    ComponentWrapper& explosionEffect = m_World->AttachComponent(newEntity, "ExplosionEffect");
    ComponentWrapper& lifeTime = m_World->AttachComponent(newEntity, "Lifetime");
    (glm::vec3)explosionEffect["ExplosionOrigin"] = (glm::vec3)e.Player["Transform"]["Position"];
    (double)explosionEffect["TimeSinceDeath"] = 0.0;
    (double)explosionEffect["ExplosionDuration"] = 8.0;
    (glm::vec4)explosionEffect["EndColor"] = glm::vec4(0, 0, 0, 1);
    (bool)explosionEffect["Randomness"] = false;
    (double)explosionEffect["RandomnessScalar"] = 1.0;
    (glm::vec2)explosionEffect["Velocity"] = glm::vec2(0.1f, 0.1f);
    (bool)explosionEffect["ColorByDistance"] = false;
    (bool)explosionEffect["ExponentialAccelaration"] = false;
    lifeTime["Lifetime"] = (double)8.0;

    //create a camera (with lifetime) slightly above the player and look down at the player
    auto cameraEntity = m_World->CreateEntity();
    ComponentWrapper& thirdPersonCameraLifeTime = m_World->AttachComponent(cameraEntity, "Lifetime");
    ComponentWrapper& thirdPersonCamera = m_World->AttachComponent(cameraEntity, "Camera");
    ComponentWrapper& thirdPersonCameraTransform = m_World->AttachComponent(cameraEntity, "Transform");
    auto pos = (glm::vec3)e.Player["Transform"]["Position"];
    pos.y += 10.0f;
    thirdPersonCameraTransform["Position"] = (glm::vec3)pos;
    //http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
    thirdPersonCameraTransform["Orientation"] = glm::vec3(-0.7f, 1.46f, 0.8f);
    thirdPersonCamera["FOV"] = 120.0;
    thirdPersonCamera["NearClip"] = 0.1;
    thirdPersonCamera["FarClip"] = 10000.0;
    thirdPersonCameraLifeTime["Lifetime"] = (double)1.0;

    //set 3rd person camera
    Events::SetCamera eSetCamera;
    eSetCamera.CameraEntity = EntityWrapper(m_World, cameraEntity);
    m_EventBroker->Publish(eSetCamera);

    //on deathanim done -> del entity
    m_World->DeleteEntity(e.Player.ID);
    return true;
}
