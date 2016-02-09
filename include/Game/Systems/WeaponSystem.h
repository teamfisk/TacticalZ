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
#include "Core/EntityFile.h"
#include "Core/EntityFileParser.h"

#include <tuple>
#include <vector>


class WeaponSystem : public ImpureSystem
{
public:
    WeaponSystem(SystemParams params, IRenderer* renderer);

    virtual void Update(double dt) override;

private:
    IRenderer* m_Renderer;

    // Events
    EventRelay<WeaponSystem, Events::Shoot> m_EShoot;
    bool WeaponSystem::OnShoot(Events::Shoot& e);
    EventRelay<WeaponSystem, Events::InputCommand> m_EInputCommand;
    bool WeaponSystem::OnInputCommand(Events::InputCommand& e);
};

#endif