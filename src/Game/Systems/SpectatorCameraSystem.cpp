#include "Systems/SpectatorCameraSystem.h"
#include "Rendering/ESetCamera.h"

SpectatorCameraSystem::SpectatorCameraSystem(SystemParams params)
    : System(params)
    , m_CamSetToTeamPick(false)
    , m_PickedTeam(-1)
{
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &SpectatorCameraSystem::OnInputCommand);
}

void SpectatorCameraSystem::Update(double dt)
{
    if (!m_CamSetToTeamPick && IsClient) {
        // Find the class pick camera and set them to it, since they need to pick a team before they can leave the screen.
        EntityWrapper spectatorCam = m_World->GetFirstEntityByName("PickTeamCamera");
        if (spectatorCam.Valid() && spectatorCam.HasComponent("Camera")) {
            m_CamSetToTeamPick = true;
            Events::SetCamera eSetCamera;
            eSetCamera.CameraEntity = spectatorCam;
            m_EventBroker->Publish(eSetCamera);
        }
    }
}

bool SpectatorCameraSystem::OnInputCommand(const Events::InputCommand& e)
{
    // Only the client should do this, and only if player is not spawned.
    if (!IsClient || LocalPlayer.Valid()) {
        return false;
    }
    bool swapToClass = e.Command == "PickTeam" || e.Command == "SwapToClassPick";
    if (e.Value == 0 || !swapToClass && e.Command != "SwapToTeamPick" && e.Command != "PickClass") {
        return false;
    }

    if (e.Command == "PickTeam") {
        m_PickedTeam = e.Value;
    }

    // If a team has not been picked, they may not exit the pick team screen.
    if (m_PickedTeam == -1) {
        return false;
    }

    // A dead client should be able to swap to and between the overwatch cameras.
    std::string camName;
    // TODO: 1 Signifies spectator, should probably have real enum here later.
    // Spectators should never end up at the class select, instead put them at the SpectatorCamera.
    if (swapToClass && m_PickedTeam != 1) {
        camName = "PickClassCamera";
    } else if (e.Command == "SwapToTeamPick") {
        camName = "PickTeamCamera";
    } else {
        camName = "SpectatorCamera";
    }
    EntityWrapper spectatorCam = m_World->GetFirstEntityByName(camName);
    // Set the camera as active, if it exists.
    if (spectatorCam.Valid() && spectatorCam.HasComponent("Camera")) {
        Events::SetCamera eSetCamera;
        eSetCamera.CameraEntity = spectatorCam;
        m_EventBroker->Publish(eSetCamera);
    }

    return true;
}