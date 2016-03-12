#ifndef EndScreenSystem_h__
#define EndScreenSystem_h__

#include "Core/System.h"
#include "Input/EInputCommand.h"
#include "Rendering/ESetCamera.h"
#include "Core/EWin.h"

class EndScreenSystem : public ImpureSystem
{
public:
    EndScreenSystem(SystemParams params);

    virtual void Update(double dt) override;

private:
    EventRelay<EndScreenSystem, Events::Win> m_EWin;
    bool OnWin(const Events::Win& e);
};

#endif