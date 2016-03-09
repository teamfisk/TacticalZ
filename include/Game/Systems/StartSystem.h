#ifndef StartSystem_h__
#define StartSystem_h__

#include "Core/System.h"
#include "Core/ResourceManager.h"
#include "Core/Event.h"
#include "Core/EventBroker.h"

class StartSystem : public ImpureSystem
{
public:
    StartSystem(SystemParams params);
    virtual void Update(double dt) override;

private:
};

#endif