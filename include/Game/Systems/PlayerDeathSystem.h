#ifndef PlayerDeathSystem_h__
#define PlayerDeathSystem_h__

#include "Core/System.h"
#include "Input/EInputCommand.h"
#include "GLM.h"
#include "Rendering/ESetCamera.h"
#include "Core/ConfigFile.h"

#include "Core/EntityFile.h"
#include "Core/EntityFileParser.h"

#include "Core/EPlayerDeath.h"

class PlayerDeathSystem : public ImpureSystem
{
public:
    PlayerDeathSystem(SystemParams params);

    virtual void Update(double dt) override;

private:
    EventRelay<PlayerDeathSystem, Events::PlayerDeath> m_OnPlayerDeath;
    bool OnPlayerDeath(Events::PlayerDeath& e);

    void createDeathEffect(EntityWrapper player);

};
#endif