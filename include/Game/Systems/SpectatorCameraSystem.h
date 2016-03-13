#ifndef SpectatorCameraSystem_h__
#define SpectatorCameraSystem_h__

#include "Core/System.h"
#include "Input/EInputCommand.h"
#include "Network/EPlayerDisconnected.h"
#include "Game/Events/EReset.h"

class SpectatorCameraSystem : public ImpureSystem
{
public:
    SpectatorCameraSystem(SystemParams params);

    virtual void Update(double dt) override;

private:
    int m_PickedTeam;
    bool m_CamSetToTeamPick;
    void reset();

    EventRelay<SpectatorCameraSystem, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);
    EventRelay<SpectatorCameraSystem, Events::PlayerDisconnected> m_EDisconnect;
    bool OnDisconnect(const Events::PlayerDisconnected& e);
    EventRelay<SpectatorCameraSystem, Events::Reset> m_EReset;
    bool OnReset(const Events::Reset& e);
};

#endif