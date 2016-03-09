#ifndef DamageIndicatorSystem_h__
#define DamageIndicatorSystem_h__

#include "Core/System.h"
#include "Core/Transform.h"
#include "Core/ResourceManager.h"
#include "Core/EntityFile.h"
#include "Core/EPlayerDamage.h"
#include "Common.h"
#include <tuple>

#include "Rendering/ESetCamera.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "Rendering/Util/CommonFunctions.h"
//#define INDICATOR_TEST
#include "Core/ConfigFile.h"

class DamageIndicatorSystem : public ImpureSystem
{
public:
    DamageIndicatorSystem(SystemParams params);
    virtual void Update(double dt) override;

private:
    EventRelay<DamageIndicatorSystem, Events::PlayerDamage> m_EPlayerDamage;
    bool OnPlayerDamage(Events::PlayerDamage& e);

    EventRelay<DamageIndicatorSystem, Events::SetCamera> m_ESetCamera;
    bool OnSetCamera(const Events::SetCamera& e);

    EntityID m_CurrentCamera = -1;
    struct DamageIndicatorStruct {
        EntityWrapper spriteEntity;
        glm::vec3 enemyPosition;
        DamageIndicatorStruct(EntityWrapper sprite, glm::vec3 pos)
            : spriteEntity(sprite)
            , enemyPosition(pos) {}
    };
    std::vector<DamageIndicatorStruct> updateDamageIndicatorVector;
    float CalculateAngle(EntityWrapper player, glm::vec3 enemyPos);

    bool m_NetworkEnabled;

    //for tests
#ifdef INDICATOR_TEST
    glm::vec3 DamageIndicatorTest(EntityWrapper player);
    int m_TestVar = 0;
#endif
};
#endif
