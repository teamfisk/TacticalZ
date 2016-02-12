#ifndef HealthSystem_h__
#define HealthSystem_h__

#include <GLFW/glfw3.h>
#include <glm/common.hpp>

#include "Common.h"
#include "Core/System.h"
#include "Core/EPlayerDamage.h"
#include "Core/EPlayerHealthPickup.h"
#include "Core/EPlayerDeath.h"
#include "Core/ConfigFile.h"
#include "Input/EInputCommand.h"

#include <tuple>
#include <vector>
#include <algorithm>

class HealthSystem : public PureSystem
{
public:
    HealthSystem(SystemParams params);

    //updatecomponent
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt) override;

private:
    bool m_NetworkEnabled;

    //methods which will take care of specific events
    EventRelay<HealthSystem, Events::PlayerDamage> m_EPlayerDamage;
    bool HealthSystem::OnPlayerDamaged(Events::PlayerDamage& e);
    EventRelay<HealthSystem, Events::PlayerHealthPickup> m_EPlayerHealthPickup;
    bool HealthSystem::OnPlayerHealthPickup(Events::PlayerHealthPickup& e);
    EventRelay<HealthSystem, Events::InputCommand> m_InputCommand;
    bool HealthSystem::OnInputCommand(Events::InputCommand& e);
    
    //vector which will keep track of health changes
    std::vector<std::tuple<EntityID, double>> m_DeltaHealthVector;

};

#endif