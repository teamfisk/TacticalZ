#ifndef PickupSpawnSystem_h__
#define PickupSpawnSystem_h__

#include "Core/System.h"
#include "Core/Transform.h"
#include "Core/ResourceManager.h"
#include "Core/EntityFileParser.h"
#include "Core/EPickupSpawned.h"
#include "Core/EPlayerHealthPickup.h"
#include "Engine/Collision/ETrigger.h"
#include "Common.h"
#include <tuple>

class PickupSpawnSystem : public ImpureSystem
{
public:
    PickupSpawnSystem(World* world, EventBroker* eventBroker);

    virtual void Update(double dt) override;

private:
    EventRelay<PickupSpawnSystem, Events::TriggerTouch> m_ETriggerTouch;
    bool OnTriggerTouch(Events::TriggerTouch& e);

    struct NewHealthPickup {
        glm::vec3 Pos;
        double HealthGain;
        double RespawnTimer;
        double DecreaseThisRespawnTimer;
    };
    std::vector<NewHealthPickup> m_ETriggerTouchVector;
};
#endif
