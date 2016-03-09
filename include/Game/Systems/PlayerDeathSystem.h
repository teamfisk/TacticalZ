#ifndef PlayerDeathSystem_h__
#define PlayerDeathSystem_h__

#include "Core/System.h"
#include "Input/EInputCommand.h"
#include "GLM.h"
#include "Rendering/ESetCamera.h"
#include "Core/ConfigFile.h"
#include "Core/EntityFile.h"
#include "Core/EPlayerDeath.h"
#include "Core/EEntityDeleted.h"

class PlayerDeathSystem : public ImpureSystem
{
public:
    PlayerDeathSystem(SystemParams params);

    virtual void Update(double dt) override;

private:
    EntityWrapper m_LocalPlayerDeathEffect;

    EventRelay<PlayerDeathSystem, Events::PlayerDeath> m_OnPlayerDeath;
    bool OnPlayerDeath(Events::PlayerDeath& e);
    EventRelay<PlayerDeathSystem, Events::EntityDeleted> m_EEntityDeleted;
    bool OnEntityDeleted(Events::EntityDeleted& e);
    EventRelay<PlayerDeathSystem, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(Events::InputCommand& e);

    void setSpectatorCamera();
    void createDeathEffect(EntityWrapper player);

};
#endif