#ifndef PickupSpawnSystem_h__
#define PickupSpawnSystem_h__

#include "Core/System.h"
#include "Core/TransformSystem.h"
#include "Core/ResourceManager.h"
#include "Core/EntityFile.h"
#include "Core/EPickupSpawned.h"
#include "Core/EPlayerHealthPickup.h"
#include "Engine/Collision/ETrigger.h"
#include "Common.h"

class PickupSpawnSystem : public ImpureSystem
{
public:
    PickupSpawnSystem(SystemParams params);

    virtual void Update(double dt) override;

private:
    EventRelay<PickupSpawnSystem, Events::TriggerTouch> m_ETriggerTouch;
    bool OnTriggerTouch(Events::TriggerTouch& e);
    EventRelay<PickupSpawnSystem, Events::TriggerLeave> m_ETriggerLeave;
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
        EntityWrapper trigger;
    };
    std::vector<EntityAtMaxValuePickupStruct> m_PickupAtMaximum;
    void DoPickup(EntityWrapper &player, EntityWrapper &trigger);
};
#endif
