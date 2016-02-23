#include "Systems/DamageIndicatorSystem.h"

DamageIndicatorSystem::DamageIndicatorSystem(SystemParams params)
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerDamage, &DamageIndicatorSystem::OnPlayerDamage);
    //current camera
    EVENT_SUBSCRIBE_MEMBER(m_ESetCamera, &DamageIndicatorSystem::OnSetCamera);

    //load texture to cache
    auto texture = CommonFunctions::LoadTexture("Textures/DamageIndicator.png", false);
    auto entityFile = ResourceManager::Load<EntityFile>("Schema/Entities/DamageIndicator.xml");
}

bool DamageIndicatorSystem::OnPlayerDamage(Events::PlayerDamage& e)
{
    //TEST
    auto currentPos = (glm::vec3)e.Victim["Transform"]["Position"];
    auto nextPos = glm::vec3(currentPos.x + 2.0f, currentPos.y, currentPos.z);
    e.Inflictor["Transform"]["Position"] = nextPos;
       
    //SPAWN
    //load the explosioneffect XML
    auto deathEffect = ResourceManager::Load<EntityFile>("Schema/Entities/PlayerDeathExplosionWithCamera.xml");
    EntityFileParser parser2(deathEffect);
    EntityID deathEffectID = parser2.MergeEntities(m_World);
    EntityWrapper deathEffectEW = EntityWrapper(m_World, deathEffectID);

    LOG_INFO("<- effect camera");

    //components that we need from player
    auto playerCamera = e.Victim.FirstChildByName("Camera");
    auto playerModel = e.Victim.FirstChildByName("PlayerModel");
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
    deathEffectEW["Transform"]["Position"] = (glm::vec3)e.Inflictor["Transform"]["Position"];
    deathEffectEW["Transform"]["Orientation"] = (glm::vec3)e.Victim["Transform"]["Orientation"];



    //TEST END


    LOG_INFO("<- damageindicator");

    if (m_CurrentCamera == EntityID_Invalid) {
        return false;
    }

    //if (e.Victim != LocalPlayer) {
    //    return false;
    //}
    if (e.Victim.Valid() && e.Victim != LocalPlayer && !e.Victim.IsChildOf(LocalPlayer)) {
        return false;
    }

    if (!e.Inflictor.Valid() || !e.Victim.Valid()) {
        return false; 
    }
    LOG_INFO("- damageindicator");

    //grab players direction
    auto playerOrientation = glm::quat((glm::vec3)e.Victim["Transform"]["Orientation"]);

    //get the position vectors, but ignore the y-height
    auto enemyPosition = (glm::vec3)e.Inflictor["Transform"]["Position"];
    auto playerPosition = (glm::vec3)e.Victim["Transform"]["Position"];
    enemyPosition.y = 0.0f;
    playerPosition.y = 0.0f;

    //calculate the enemy to player vector
    auto enemyPlayerVector = glm::normalize(playerPosition - enemyPosition);

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
    auto entityFile = ResourceManager::Load<EntityFile>("Schema/Entities/DamageIndicator.xml");
    EntityFileParser parser(entityFile);
    EntityID spriteID = parser.MergeEntities(m_World);
    m_World->SetParent(spriteID, m_CurrentCamera);
    auto spriteWrapper = EntityWrapper(m_World, spriteID);
    //simply set the rotation z-wise to the angleBetweenVectors
    spriteWrapper["Transform"]["Orientation"] = glm::vec3(0, 0, angleBetweenVectors);

    LOG_INFO("-> damageindicator");

    return true;
}

bool DamageIndicatorSystem::OnSetCamera(const Events::SetCamera& e) {
    m_CurrentCamera = e.CameraEntity.ID;
    return true;
}
