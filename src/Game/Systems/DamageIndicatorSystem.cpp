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

void DamageIndicatorSystem::Update(double dt) {
    if (!IsServer) {
        for (auto& iter = updateDamageIndicatorVector.begin(); iter != updateDamageIndicatorVector.end(); iter++) {
            if (!iter->spriteEntity.Valid())
                //iter->timeLeft -= -dt;
                /*if (iter->timeLeft < 0.0)*/ {
                    updateDamageIndicatorVector.erase(iter);
                    break;
            }
            auto angleBetweenVectors = CalculateAngle(LocalPlayer, iter->enemyPosition);
            //simply set the rotation z-wise to the angleBetweenVectors
            iter->spriteEntity["Transform"]["Orientation"] = glm::vec3(0, 0, angleBetweenVectors);
        }
    }
}

bool DamageIndicatorSystem::OnPlayerDamage(Events::PlayerDamage& e)
{
    //TEST
    auto currentPos = (glm::vec3)e.Victim["Transform"]["Position"];

    auto testTempo = 1;
    auto testTempo2 = 1;
    if (m_TestVar % 4 == 0) {
        testTempo = -1;
        testTempo2 = 1;
    }
    if (m_TestVar % 4 == 1) {
        testTempo = 1;
        testTempo2 = 1;
    }
    if (m_TestVar % 4 == 2) {
        testTempo *= -1;
        testTempo2 = -1;
    }
    if (m_TestVar % 4 == 3) {
        testTempo = 1;
        testTempo2 = -1;
    }
    m_TestVar++;
    auto inflictorPos = glm::vec3(currentPos.x + testTempo*12.0f, currentPos.y, currentPos.z + testTempo2*12.0f);
    //inflictor = real entity...

    //SPAWN
    //load the explosioneffect XML
    auto deathEffect = ResourceManager::Load<EntityFile>("Schema/Entities/PlayerDeathExplosionWithCamera.xml");
    EntityFileParser parser2(deathEffect);
    EntityID deathEffectID = parser2.MergeEntities(m_World);
    EntityWrapper deathEffectEW = EntityWrapper(m_World, deathEffectID);

    //components that we need from player
    auto playerModel = e.Victim.FirstChildByName("PlayerModel");
    auto playerEntityModel = playerModel["Model"];
    auto playerEntityAnimation = playerModel["Animation"];

    //copy the data from player to explosioneffectmodel
    playerEntityModel.Copy(deathEffectEW["Model"]);
    playerEntityAnimation.Copy(deathEffectEW["Animation"]);

    //copy the models position,orientation
    deathEffectEW["Transform"]["Position"] = inflictorPos;
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
    auto enemyPosition = inflictorPos;
    auto playerPosition = (glm::vec3)e.Victim["Transform"]["Position"];
    enemyPosition.y = 0.0f;
    playerPosition.y = 0.0f;

    //calculate the enemy to player vector
    auto enemyPlayerVector = glm::normalize(playerPosition - enemyPosition);

    //get angle from players current rotation, this angle is how much you rotate around the y-axis
    //auto playerAngle = glm::angle(playerOrientation);
    //auto playerRotationVector = glm::normalize(glm::rotateY(glm::vec3(0, 0, 1), playerAngle));
    auto rotationVector = glm::toMat4(Transform::AbsoluteOrientation(e.Victim))*glm::vec4(0, 0, 1, 0);
    auto playerRotationVector = glm::vec3(rotationVector);
    auto rotVNinety = glm::rotateY(rotationVector, 1.57f);
    auto playerSideVector = glm::vec3(rotVNinety);

    //dot product of players direction-vector and enemys-to-playervector will give the cos of the angle between the vectors
    auto playerRotationDot = glm::dot(playerRotationVector, enemyPlayerVector);
    //to get the angle between the vectors just do cos-inverse
    auto angleBetweenVectors = glm::acos(playerRotationDot);

    //rotate the direction-vector 90 degrees to get the players side-vector
    //auto playerSideVector = glm::normalize(glm::rotateY(glm::vec3(0, 0, 1), playerAngle + 1.57f));
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

    updateDamageIndicatorVector.emplace_back(spriteWrapper, enemyPosition);

    LOG_INFO("-> damageindicator");

    return true;
}

bool DamageIndicatorSystem::OnSetCamera(const Events::SetCamera& e) {
    m_CurrentCamera = e.CameraEntity.ID;
    return true;
}
float DamageIndicatorSystem::CalculateAngle(EntityWrapper player, glm::vec3 enemyPos) {
    //grab players direction
    auto playerOrientation = glm::quat((glm::vec3)player["Transform"]["Orientation"]);

    //get the position vectors, but ignore the y-height
    auto enemyPosition = enemyPos;
    auto playerPosition = (glm::vec3)player["Transform"]["Position"];
    enemyPosition.y = 0.0f;
    playerPosition.y = 0.0f;

    //calculate the enemy to player vector
    auto enemyPlayerVector = glm::normalize(playerPosition - enemyPosition);

    //get angle from players current rotation, this angle is how much you rotate around the y-axis
    //auto playerAngle = glm::angle(playerOrientation);
    //auto playerRotationVector = glm::normalize(glm::rotateY(glm::vec3(0, 0, 1), playerAngle));
    auto rotationVector = glm::toMat4(Transform::AbsoluteOrientation(player))*glm::vec4(0, 0, 1, 0);
    auto playerRotationVector = glm::vec3(rotationVector);
    auto rotVNinety = glm::rotateY(rotationVector, 1.57f);
    auto playerSideVector = glm::vec3(rotVNinety);

    //dot product of players direction-vector and enemys-to-playervector will give the cos of the angle between the vectors
    auto playerRotationDot = glm::dot(playerRotationVector, enemyPlayerVector);
    //to get the angle between the vectors just do cos-inverse
    auto angleBetweenVectors = glm::acos(playerRotationDot);

    //rotate the direction-vector 90 degrees to get the players side-vector
    //auto playerSideVector = glm::normalize(glm::rotateY(glm::vec3(0, 0, 1), playerAngle + 1.57f));
    //dot of sidevector positive = enemy is on the right side, dot sidevector negative = left side
    auto playerSideVectorDot = glm::dot(playerSideVector, enemyPlayerVector);
    if (playerSideVectorDot < 0) {
        angleBetweenVectors = -angleBetweenVectors;
    }

    return angleBetweenVectors;
}
