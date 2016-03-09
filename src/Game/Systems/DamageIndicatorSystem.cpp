#include "Systems/DamageIndicatorSystem.h"

DamageIndicatorSystem::DamageIndicatorSystem(SystemParams params)
    : System(params)
{
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerDamage, &DamageIndicatorSystem::OnPlayerDamage);
    //current camera
    EVENT_SUBSCRIBE_MEMBER(m_ESetCamera, &DamageIndicatorSystem::OnSetCamera);

    //load texture to cache
    auto texture = CommonFunctions::LoadTexture("Textures/DamageIndicator.png", false);
    auto entityFile = ResourceManager::Load<EntityXMLFile>("Schema/Entities/DamageIndicator.xml");
    m_NetworkEnabled = ResourceManager::Load<ConfigFile>("Config.ini")->Get("Networking.StartNetwork", false);
}

void DamageIndicatorSystem::Update(double dt) {
    if ((!IsServer || !m_NetworkEnabled) && LocalPlayer.Valid()) {
        for (auto& iter = updateDamageIndicatorVector.begin(); iter != updateDamageIndicatorVector.end(); iter++) {
            if (!iter->spriteEntity.Valid()) {
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
    if (m_CurrentCamera == EntityID_Invalid) {
        return false;
    }

    if (e.Victim.Valid() && e.Victim != LocalPlayer && !e.Victim.IsChildOf(LocalPlayer)) {
        return false;
    }

    if (!e.Inflictor.Valid() || !e.Victim.Valid()) {
        return false;
    }

    //friendly fire - return
    if (e.Damage < 0.1f) {
        return false;
    }

    glm::vec3 inflictorPos = e.Inflictor["Transform"]["Position"];
    //if testing
#ifdef INDICATOR_TEST
    inflictorPos = DamageIndicatorTest(e.Victim);
#endif

    float angleBetweenVectors = CalculateAngle(e.Victim, inflictorPos);

    //load & set the "2d" sprite
    auto entityFile = ResourceManager::Load<EntityFile>("Schema/Entities/DamageIndicator.xml");
    EntityWrapper sprite = entityFile->MergeInto(m_World);
    m_World->SetParent(sprite.ID, m_CurrentCamera);
    //simply set the rotation z-wise to the angleBetweenVectors
    sprite["Transform"]["Orientation"] = glm::vec3(0, 0, angleBetweenVectors);

    if (!IsServer || !m_NetworkEnabled) {
        updateDamageIndicatorVector.emplace_back(sprite, inflictorPos);
    }

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

    //get the rotationvector relative to the z-axis
    auto rotationVectorVec3 = glm::vec3(glm::toMat4(Transform::AbsoluteOrientation(player))*glm::vec4(0, 0, 1, 0));
    //rotate the direction-vector 90 degrees to get the players side-vector
    auto playerSideVector = glm::vec3(glm::rotateY(rotationVectorVec3, 1.57f));

    //dot product of players direction-vector and enemys-to-playervector will give the cos of the angle between the vectors
    auto playerRotationDot = glm::dot(rotationVectorVec3, enemyPlayerVector);
    //to get the angle between the vectors just do cos-inverse
    auto angleBetweenVectors = glm::acos(playerRotationDot);

    //dot of sidevector positive = enemy is on the right side, dot sidevector negative = left side
    auto playerSideVectorDot = glm::dot(playerSideVector, enemyPlayerVector);
    if (playerSideVectorDot < 0) {
        angleBetweenVectors = -angleBetweenVectors;
    }

    return angleBetweenVectors;
}
#ifdef INDICATOR_TEST
glm::vec3 DamageIndicatorSystem::DamageIndicatorTest(EntityWrapper player) {
    auto currentPos = (glm::vec3)player["Transform"]["Position"];

    auto testVar = 1;
    auto testVar2 = 1;
    if (m_TestVar % 4 == 0) {
        testVar = -1;
        testVar2 = 1;
    }
    if (m_TestVar % 4 == 1) {
        testVar = 1;
        testVar2 = 1;
    }
    if (m_TestVar % 4 == 2) {
        testVar *= -1;
        testVar2 = -1;
    }
    if (m_TestVar % 4 == 3) {
        testVar = 1;
        testVar2 = -1;
    }
    m_TestVar++;

    auto inflictorPos = glm::vec3(currentPos.x + testVar*6.0f, currentPos.y, currentPos.z + testVar2*6.0f);

    //load the explosioneffect XML
    auto deathEffect = ResourceManager::Load<EntityXMLFile>("Schema/Entities/PlayerDeathExplosionWithCamera.xml");
    EntityXMLFileParser parser(deathEffect);
    EntityID deathEffectID = parser.MergeEntities(m_World);
    EntityWrapper deathEffectEW = EntityWrapper(m_World, deathEffectID);

    //components that we need from player
    auto playerModel = player.FirstChildByName("PlayerModel");
    auto playerEntityModel = playerModel["Model"];
    auto playerEntityAnimation = playerModel["Animation"];

    //copy the data from player to explosioneffectmodel
    playerEntityModel.Copy(deathEffectEW["Model"]);
    playerEntityAnimation.Copy(deathEffectEW["Animation"]);

    //copy the models position,orientation
    deathEffectEW["Transform"]["Position"] = inflictorPos;
    deathEffectEW["Transform"]["Orientation"] = (glm::vec3)player["Transform"]["Orientation"];
    return inflictorPos;
}
#endif