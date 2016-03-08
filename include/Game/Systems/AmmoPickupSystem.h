#ifndef AmmoPickupSystem_h__
#define AmmoPickupSystem_h__

#include "Core/System.h"
#include "Core/Transform.h"
#include "Core/ResourceManager.h"
#include "Core/EntityFile.h"
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

    EventRelay<AmmoPickupSystem, Events::AmmoPickup> m_EAmmoPickup;
    bool OnAmmoPickup(Events::AmmoPickup& e);

    struct NewAmmoPickup {
        glm::vec3 Pos;
        double AmmoGain;
        double RespawnTimer;
        double DecreaseThisRespawnTimer;
        EntityID parentID;
    };
    std::vector<NewAmmoPickup> m_ETriggerTouchVector;
    struct EntityAtMaxValuePickupStruct {
        EntityWrapper player;
        EntityWrapper trigger;
    };
    std::vector<EntityAtMaxValuePickupStruct> m_PickupAtMaximum;
    void DoPickup(EntityWrapper &player, EntityWrapper &trigger);
    //class
    enum class PlayerClass {
        Assault,
        Defender,
        Sniper,
        None
    };
    //helper methods
    bool DoesPlayerHaveMaxAmmo(EntityWrapper &player);
    PlayerClass DetermineClass(EntityWrapper &player);
    int& GetPlayerAmmo(EntityWrapper &player);
    int GetPlayerMaxAmmo(EntityWrapper &player);
};
#endif
