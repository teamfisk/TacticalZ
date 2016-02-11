#ifndef WeaponSystem_h__
#define WeaponSystem_h__

#include "Rendering/IRenderer.h"

#include "Common.h"
#include "Core/System.h"
#include "Core/EPlayerDamage.h"
#include "Core/EShoot.h"
#include "Core/EPlayerSpawned.h"
#include "Input/EInputCommand.h"
#include "Core/EntityFile.h"
#include "Core/EntityFileParser.h"
#include "Core/Octree.h"
#include "Collision/EntityAABB.h"
#include "Systems/SpawnerSystem.h"
#include "Sound/EPlaySoundOnEntity.h"
#include "AssaultWeaponBehaviour.h"

class WeaponSystem : public PureSystem, ImpureSystem
{
public:
    WeaponSystem(SystemParams params, IRenderer* renderer, Octree<EntityAABB>* collisionOctree);

    virtual void Update(double dt) override;
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cPlayer, double dt) override;

private:
    SystemParams m_SystemParams;
    IRenderer* m_Renderer;
    Octree<EntityAABB>* m_CollisionOctree;

    std::unordered_map<EntityWrapper, std::shared_ptr<WeaponBehaviour>> m_ActiveWeapons;

    // Events
    EventRelay<WeaponSystem, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawned(Events::PlayerSpawned& e);
    EventRelay<WeaponSystem, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(Events::InputCommand& e);

    void selectWeapon(EntityWrapper player, ComponentInfo::EnumType slot);
};

#endif