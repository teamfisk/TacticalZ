#include "Systems/DamageIndicatorSystem.h"

DamageIndicatorSystem::DamageIndicatorSystem(World* m_World, EventBroker* eventBroker)
    : System(m_World, eventBroker)
{
    EVENT_SUBSCRIBE_MEMBER(m_DamageTakenFromPlayer, &DamageIndicatorSystem::OnPlayerDamageTaken);
    //current camera
    EVENT_SUBSCRIBE_MEMBER(m_ESetCamera, &DamageIndicatorSystem::OnSetCamera);
}

bool DamageIndicatorSystem::OnPlayerDamageTaken(Events::PlayerDamage& e)
{
    if (m_CurrentCamera == -1) {
        return false;
    }

    //grab players direction
    auto playerOrientation = glm::quat((glm::vec3)e.Player["Transform"]["Orientation"]);

    //get the position vectors, but ignore the y-height
    auto enemyPosition = (glm::vec3) e.PlayerShooter["Transform"]["Position"];
    auto playerPosition = (glm::vec3) e.Player["Transform"]["Position"];
    enemyPosition.y = 0.0f;
    playerPosition.y = 0.0f;

    //calculate the enemy to player vector
    auto enemyPlayerVector = glm::normalize((glm::vec3) playerPosition - enemyPosition);

    //get angle from players current rotation, this angle is how much you rotate around the y-axis
    auto playerAngle = glm::angle(playerOrientation);
    auto playerRotationVector = glm::normalize(glm::rotateY(glm::vec3(0, 0, 1), playerAngle));

    //dot product of players direction-vector and enemys-to-playervector will give the cos of the angle between the vectors
    auto playerRotationDot = glm::dot(playerRotationVector, enemyPlayerVector);
    //to get the angle between the vectors just do cos-inverse
    auto angleBetweenVectors = glm::acos(playerRotationDot);

    //rotate the direction-vector 90 degrees to get the players side-vector
    auto playerSideVector = glm::normalize(glm::rotateY(glm::vec3(0, 0, 1), playerAngle + 1.57f));
    //dot of sidevector positive = enemy is on the right side, dot sidevector negative = left side
    auto playerSideVectorDot = glm::dot(playerSideVector, enemyPlayerVector);
    if (playerSideVectorDot < 0) {
        angleBetweenVectors = -angleBetweenVectors;
    }

    //load & set the "2d" sprite
    auto entityFile = ResourceManager::Load<EntityFile>("Schema/Entities/SpriteTestTemporary.xml");
    EntityFileParser parser(entityFile);
    EntityID spriteID = parser.MergeEntities(m_World);
    m_World->SetParent(spriteID, m_CurrentCamera);
    auto spriteWrapper = EntityWrapper(m_World, spriteID);
    //simply set the rotation z-wise to the angleBetweenVectors
    spriteWrapper["Transform"]["Orientation"] = glm::vec3(0, 0, angleBetweenVectors);

    return true;
}

bool DamageIndicatorSystem::OnSetCamera(const Events::SetCamera& e) {
    m_CurrentCamera = e.CameraEntity.ID;
    return true;
}