#ifndef DamageIndicatorSystem_h__
#define DamageIndicatorSystem_h__

#include "Core/System.h"
#include "Core/Transform.h"
#include "Core/ResourceManager.h"
#include "Core/EntityFileParser.h"
#include "Core/EPickupSpawned.h"
#include "Core/EPlayerDamage.h"
#include "Engine/Collision/ETrigger.h"
#include "Common.h"
#include <tuple>

//temp
#include "Input/EInputCommand.h"
#include "Rendering/ESetCamera.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>

class DamageIndicatorSystem : public ImpureSystem
{
public:
    DamageIndicatorSystem(World* world, EventBroker* eventBroker);

    virtual void Update(double dt) override;

private:

    EventRelay<DamageIndicatorSystem, Events::PlayerDamage> m_DamageTakenFromPlayer;
    bool OnPlayerDamageTaken(Events::PlayerDamage& e);

    //temp
    EventRelay<DamageIndicatorSystem, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(Events::InputCommand& e);

    EventRelay<DamageIndicatorSystem, Events::SetCamera> m_ESetCamera;
    bool OnSetCamera(const Events::SetCamera& e);

    EntityID m_CurrentCamera = -1;
};
#endif
