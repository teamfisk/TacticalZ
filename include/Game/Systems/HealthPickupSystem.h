#ifndef HealthPickupSystem_h__
#define HealthPickupSystem_h__

#include "Core/System.h"
#include "Core/Transform.h"
#include "Core/ResourceManager.h"
#include "Core/EntityFileParser.h"
#include "Core/EPickupSpawned.h"
#include "Core/EPlayerHealthPickup.h"
#include "Engine/Collision/ETrigger.h"
#include "Common.h"
#include <tuple>
#include "Core/ConfigFile.h"

class HealthPickupSystem : public ImpureSystem
{
public:
    HealthPickupSystem(SystemParams params);

    virtual void Update(double dt) override;

private:
    EventRelay<HealthPickupSystem, Events::TriggerTouch> m_ETriggerTouch;
    bool OnTriggerTouch(Events::TriggerTouch& e);
    EventRelay<HealthPickupSystem, Events::TriggerLeave> m_ETriggerLeave;
    bool OnTriggerLeave(Events::TriggerLeave& e);

    struct NewHealthPickup {
        glm::vec3 Pos;
        double HealthGain;
        double RespawnTimer;
        double DecreaseThisRespawnTimer;
        EntityID parentID;
    };
    std::vector<NewHealthPickup> m_ETriggerTouchVector;
    struct EntityAtMaxValuePickupStruct {
        EntityWrapper player;
        NewHealthPickup pickup;
        EntityWrapper trigger;
    };
    std::vector<EntityAtMaxValuePickupStruct> m_PickupAtMaximum;
    void DoPickup(EntityWrapper &player, EntityWrapper &trigger);
    void DoPickup(EntityWrapper &player, NewHealthPickup &triggerCopy, EntityWrapper &trigger);
};
#endif
