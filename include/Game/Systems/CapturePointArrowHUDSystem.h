#ifndef CapturePointArrowHUDSystem_h__
#define CapturePointArrowHUDSystem_h__

#include <GLFW/glfw3.h>
#include <glm/common.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "Common.h"
#include "Core/System.h"
#include "Core/TransformSystem.h"
#include "Core/ECaptured.h"


class CapturePointArrowHUDSystem : public ImpureSystem
{
public:
    CapturePointArrowHUDSystem(SystemParams params);

    virtual void Update(double dt) override;

private:
    EventRelay<CapturePointArrowHUDSystem, Events::Captured> m_ECapturedEvent;
    bool OnCapturePointCaptured(Events::Captured& e);

    bool m_InitialtargetsSet = false;
    glm::vec3 m_RedTeamCurrentTarget;
    glm::vec3 m_BlueTeamCurrentTarget;
};

#endif