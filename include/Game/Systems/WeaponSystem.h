#ifndef WeaponSystem_h__
#define WeaponSystem_h__

//#include <GLFW/glfw3.h>
//#include <glm/common.hpp>
#include "Rendering/IRenderer.h"

#include "Common.h"
#include "Core/System.h"
#include "Core/EPlayerDamage.h"
#include "Core/EShoot.h"
#include "Input/EInputCommand.h"

#include <tuple>
#include <vector>


class WeaponSystem : public ImpureSystem
{
public:
    WeaponSystem(World* world, EventBroker* eventBroker, IRenderer* renderer);

    virtual void Update(double dt) override;

private:
    //methods which will take care of specific events
    EventRelay<WeaponSystem, Events::Shoot> m_EShoot;
    bool WeaponSystem::OnShoot(const Events::Shoot& e);

    EventRelay<WeaponSystem, Events::InputCommand> m_EInputCommand;
    bool WeaponSystem::OnInputCommand(const Events::InputCommand& e);

    IRenderer* m_Renderer;

    std::vector<std::tuple<EntityID, glm::vec2>> m_EShootVector;
};

#endif