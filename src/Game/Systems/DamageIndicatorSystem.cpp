#include "Systems/DamageIndicatorSystem.h"

DamageIndicatorSystem::DamageIndicatorSystem(World* m_World, EventBroker* eventBroker)
    : System(m_World, eventBroker)
{
    EVENT_SUBSCRIBE_MEMBER(m_DamageTakenFromPlayer, &DamageIndicatorSystem::OnPlayerDamageTaken);
    //TEMP
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &DamageIndicatorSystem::OnInputCommand);
    //current camera
    EVENT_SUBSCRIBE_MEMBER(m_ESetCamera, &DamageIndicatorSystem::OnSetCamera);

}

void DamageIndicatorSystem::Update(double dt)
{

}


bool DamageIndicatorSystem::OnPlayerDamageTaken(Events::PlayerDamage& e)
{
    auto test1 = (glm::vec3)e.PlayerShooter["Transform"]["Orientation"];
    auto test2 = (glm::vec3)e.Player["Transform"]["Orientation"];
    //calculate direction
    auto enemyOrientation = glm::quat(((glm::vec3)e.PlayerShooter["Transform"]["Orientation"]));
    auto playerOrientation = glm::quat((glm::vec3)e.Player["Transform"]["Orientation"]);
    //calculate difference in angle (quaternion math)
    auto angle1 = glm::angle(playerOrientation);
    auto angle2 = glm::angle(enemyOrientation);
    auto quat = glm::angleAxis(angle2 - angle1, glm::vec3(0, 1, 0));
    auto vec3Orientation = glm::eulerAngles(quat);

    //load & set the "2d" sprite
    auto entityFile = ResourceManager::Load<EntityFile>("Schema/Entities/SpriteTestTemporary.xml");
    EntityFileParser parser(entityFile);
    EntityID spriteID = parser.MergeEntities(m_World);
    m_World->SetParent(spriteID, m_CurrentCamera);
    auto cameraWrapper = EntityWrapper(m_World, spriteID);
    cameraWrapper["Transform"]["Orientation"] = vec3Orientation;

    return true;
}

//TEMP
bool DamageIndicatorSystem::OnInputCommand(Events::InputCommand& e)
{
    if (e.Command != "Jump" || e.Value > 0) {
        return false;
    }
    //auto entityFile = ResourceManager::Load<EntityFile>("Schema/Entities/SpriteTestTemporary.xml");
    //EntityFileParser parser(entityFile);
    //EntityID spriteID = parser.MergeEntities(m_World);
    ////get currently active camera
    ////auto cameras = m_World->GetComponents("Camera");
    ////for (auto& cCamera : *cameras) {
    ////    
    ////    //auto temp = m_World->GetParent(cCamera.EntityID);
    ////    m_World->SetParent(spriteID, cCamera.EntityID);
    ////}
    ////m_CurrentCamera
    //m_World->SetParent(spriteID, m_CurrentCamera);

    //auto cameraWrapper = EntityWrapper(m_World, spriteID);
    //cameraWrapper["Transform"]["Orientation"] = glm::vec3(1, 1, 1);
    //ray player-enemyplayer eller bara spelarnas direction


    //TODO: life time, rotering
//den ska väl vara där hela tiden, bara det att den inte syns


    //EntityWrapper(m_World, spriteID);

    auto players = m_World->GetComponents("Player");
    EntityID id1 = (*players->begin()).EntityID;
    EntityID id2;
    int lameCounter = 0;
    for (auto& cPlayers : *players) {
        if (lameCounter == 1) {
            id2 = cPlayers.EntityID;
        }
        lameCounter++;
    }
    if (lameCounter != 2) {
        return false;
    }

    //do something here
    Events::PlayerDamage ePlayerDamage;
    ePlayerDamage.Player = EntityWrapper(m_World, id1);
    ePlayerDamage.PlayerShooter = EntityWrapper(m_World, id2);
    ePlayerDamage.Damage = 1;
    m_EventBroker->Publish(ePlayerDamage);

    return true;
}

bool DamageIndicatorSystem::OnSetCamera(const Events::SetCamera& e) {
    m_CurrentCamera = e.CameraEntity.ID;
    return true;
}