#ifndef CapturePointSystem_h__
#define CapturePointSystem_h__

#include <GLFW/glfw3.h>
#include <glm/common.hpp>

#include "Common.h"
#include "Core/System.h"
#include "Engine/Collision/ETrigger.h"
#include "Core/ECaptured.h"
#include "Core/EWin.h"

#include <tuple>
#include <vector>

class CapturePointSystem : public PureSystem
{
public:
    //WARNING: on new map, destroy all info in the vectors, as well as reset all variables (just make new?)
    CapturePointSystem(EventBroker* eventBroker);

    //updatecomponent
    virtual void UpdateComponent(World* world, EntityWrapper& entity, ComponentWrapper& capturePoint, double dt) override;

private:
    //methods which will take care of specific events
    EventRelay<CapturePointSystem, Events::TriggerTouch> m_ETriggerTouch;
    bool CapturePointSystem::OnTriggerTouch(const Events::TriggerTouch& e);
    EventRelay<CapturePointSystem, Events::TriggerLeave> m_ETriggerLeave;
    bool CapturePointSystem::OnTriggerLeave(const Events::TriggerLeave& e);
    EventRelay<CapturePointSystem, Events::Captured> m_ECaptured;
    bool CapturePointSystem::OnCaptured(const Events::Captured& e);

    bool m_WinnerWasFound = false;
    //need to track these variables for the captureSystem to work as per design!
    const int m_NotACapturePoint = 999;
    int m_RedTeamNextPossibleCapturePoint = m_NotACapturePoint;
    int m_BlueTeamNextPossibleCapturePoint = m_NotACapturePoint;
    int m_RedTeamHomeCapturePoint = m_NotACapturePoint;
    int m_BlueTeamHomeCapturePoint = m_NotACapturePoint;

    int m_NumberOfCapturePoints = 0;
    std::map<int, EntityID> m_CapturePointNumberToEntityIDMap;

    //std::vector<ComponentWrapper>

    const double m_CaptureTimeToTakeOver = 15.0;
    bool m_ResetTimers = false;

    //vectors which will keep track of enter/leave changes
    std::vector<std::tuple<EntityID, EntityID>> m_ETriggerTouchVector;
    std::vector<std::tuple<EntityID, EntityID>> m_ETriggerLeaveVector;
};

#endif