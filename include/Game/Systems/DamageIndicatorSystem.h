#ifndef DamageIndicatorSystem_h__
#define DamageIndicatorSystem_h__

#include "Core/System.h"
#include "Core/Transform.h"
#include "Core/ResourceManager.h"
#include "Core/EntityFileParser.h"
#include "Core/EPlayerDamage.h"
#include "Common.h"
#include <tuple>

#include "Rendering/ESetCamera.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "Rendering/Util/CommonFunctions.h"

class DamageIndicatorSystem : public System
{
public:
    DamageIndicatorSystem(SystemParams params);

private:
    EventRelay<DamageIndicatorSystem, Events::PlayerDamage> m_EPlayerDamage;
    bool OnPlayerDamage(Events::PlayerDamage& e);

    EventRelay<DamageIndicatorSystem, Events::SetCamera> m_ESetCamera;
    bool OnSetCamera(const Events::SetCamera& e);

    EntityID m_CurrentCamera = -1;

};
#endif
