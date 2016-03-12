#ifndef WinScreenSystem_h__
#define WinScreenSystem_h__

#include "Core/System.h"
#include "Input/EInputCommand.h"

class WinScreenSystem : public ImpureSystem
{
public:
    WinScreenSystem(SystemParams params);

    virtual void Update(double dt) override;

private:
};

#endif