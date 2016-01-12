#ifndef CapturePointSystem_h__
#define CapturePointSystem_h__

#include <GLFW/glfw3.h>
#include <glm/common.hpp>

#include "Common.h"
#include "Core/System.h"
#include "Engine/Collision/ETrigger.h"

#include <tuple>
#include <vector>

class CapturePointSystem : public PureSystem
{
public:
    //TODO: on new map, destroy all info in the vectors
    CapturePointSystem(EventBroker* eventBroker);

    //updatecomponent
    virtual void UpdateComponent(World* world, ComponentWrapper& capturePoint, double dt) override;

private:
    //methods which will take care of specific events
    EventRelay<CapturePointSystem, Events::TriggerTouch> m_ETriggerTouch;
    bool CapturePointSystem::OnTriggerTouch(const Events::TriggerTouch& e);
    EventRelay<CapturePointSystem, Events::TriggerLeave> m_ETriggerLeave;
    bool CapturePointSystem::OnTriggerLeave(const Events::TriggerLeave& e);

    //vectors which will keep track of enter/leave changes
    std::vector<std::tuple<EntityID, EntityID>> m_ETriggerTouchVector;
    std::vector<std::tuple<EntityID, EntityID>> m_ETriggerLeaveVector;
};

#endif