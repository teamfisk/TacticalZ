#ifndef CapturePointHUDSystem_h__
#define CapturePointHUDSystem_h__

#include <GLFW/glfw3.h>
#include <glm/common.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Common.h"
#include "Core/System.h"
#include "Collision/ETrigger.h"

class CapturePointHUDSystem : public ImpureSystem
{
public:
    CapturePointHUDSystem(SystemParams params);

    virtual void Update(double dt) override;

private:
};

#endif