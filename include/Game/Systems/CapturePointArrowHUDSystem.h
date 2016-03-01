#ifndef CapturePointArrowHUDSystem_h__
#define CapturePointArrowHUDSystem_h__

#include <GLFW/glfw3.h>
#include <glm/common.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "Common.h"
#include "Core/System.h"
#include "Core/Transform.h"


class CapturePointArrowHUDSystem : public ImpureSystem
{
public:
    CapturePointArrowHUDSystem(SystemParams params);

    virtual void Update(double dt) override;

private:
};

#endif