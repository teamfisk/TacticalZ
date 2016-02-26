#ifndef AmmoPickupSystem_h__
#define AmmoPickupSystem_h__

#include "Core/System.h"
#include "Core/Transform.h"
#include "Core/ResourceManager.h"
#include "Core/EntityFileParser.h"
#include "Core/EPickupSpawned.h"
#include "Core/EAmmoPickup.h"
#include "Engine/Collision/ETrigger.h"
#include "Common.h"

class AmmoPickupSystem : public ImpureSystem
{
public:
    AmmoPickupSystem(SystemParams params);

    virtual void Update(double dt) override;

private:
    EventRelay<AmmoPickupSystem, Events::TriggerTouch> m_ETriggerTouch;
    bool OnTriggerTouch(Events::TriggerTouch& e);
    EventRelay<AmmoPickupSystem, Events::TriggerLeave> m_ETriggerLeave;
    bool OnTriggerLeave(Events::TriggerLeave& e);

    struct NewPickup {
        glm::vec3 Pos;
        double AmmoGain;
        double RespawnTimer;
        double DecreaseThisRespawnTimer;
        EntityID parentID;
        bool isAmmoPickup;
    };
    struct EntityAtMaxValuePickupStruct {
        EntityWrapper player;
        EntityWrapper pickup;
    };
    std::vector<NewPickup> m_ETriggerTouchVector;
    std::vector<EntityAtMaxValuePickupStruct> m_PickupAtMaximum;
    
    void DoPickup(EntityWrapper &player, EntityWrapper &trigger);
};
#endif
