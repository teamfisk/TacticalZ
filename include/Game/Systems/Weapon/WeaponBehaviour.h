#ifndef WeaponBehaviour_h__
#define WeaponBehaviour_h__

#include "Core/System.h"
#include "Rendering/IRenderer.h"
#include "Core/Octree.h"
#include "Collision/EntityAABB.h"
#include "Input/EInputCommand.h"
#include "Systems/SpawnerSystem.h"

template <typename ETYPE>
class WeaponBehaviour : public PureSystem
{
    friend class WeaponSystem;

public:
    WeaponBehaviour(SystemParams params, std::string componentType, IRenderer* renderer, Octree<EntityAABB>* collisionOctree)
        : System(params)
        , PureSystem(componentType)
        , m_Renderer(renderer)
        , m_CollisionOctree(collisionOctree)
    {
        EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &WeaponBehaviour::_OnInputCommand)
    }
    virtual ~WeaponBehaviour() = default;

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cWeapon, double dt) override
    {
        auto weapon = getActiveWeapon(entity);
        if (!weapon) {
            return;
        } else {
            UpdateWeapon(cWeapon, *weapon, dt);
        }
    }

protected:
    struct WeaponInfo
    {
        EntityWrapper Player;
        EntityWrapper WeaponEntity;
        EntityWrapper FirstPersonEntity;
        EntityWrapper ThirdPersonEntity;
    };

    IRenderer* m_Renderer;
    Octree<EntityAABB>* m_CollisionOctree;
    std::unordered_map<EntityWrapper, WeaponInfo> m_ActiveWeapons;

    virtual void UpdateWeapon(ComponentWrapper cWeapon, WeaponInfo& wi, double dt) { }
    virtual void OnPrimaryFire(ComponentWrapper cWeapon, WeaponInfo& wi) { }
    virtual void OnCeasePrimaryFire(ComponentWrapper cWeapon, WeaponInfo& wi) { }
    virtual void OnReload(ComponentWrapper cWeapon, WeaponInfo& wi) { }
    virtual void OnHolster(ComponentWrapper cWeapon, WeaponInfo& wi) { }
    virtual bool OnInputCommand(ComponentWrapper cWeapon, WeaponInfo& wi, const Events::InputCommand& e) { return false; }

private:
    EventRelay<ETYPE, Events::InputCommand> m_EInputCommand;
    bool _OnInputCommand(const Events::InputCommand& e)
    {
        EntityWrapper player = e.Player;
        if (e.PlayerID == -1) {
            player = LocalPlayer;
        }

        // Make sure the player is alive
        if (!player.Valid()) {
            return false;
        }

        // Make sure the player has this weapon
        auto cWeapon = getWeaponComponent(player);
        if (!cWeapon) {
            return false;
        }

        // Weapon selection
        if (e.Command == "SelectWeapon") {
            if (e.Value > 0) {
                if (static_cast<ComponentInfo::EnumType>(e.Value) == static_cast<ComponentInfo::EnumType>((*cWeapon)["Slot"])) {
                    selectWeapon(player);
                } else {
                    holsterWeapon(*cWeapon, player);
                }
            }
        }

        // Only handle weapon actions if the weapon is active
        auto activeWeapon = getActiveWeapon(player);
        if (!activeWeapon) {
            return false;
        }

        // Fire
        if (e.Command == "PrimaryFire") {
            if (e.Value > 0) {
                OnPrimaryFire(*cWeapon, *activeWeapon);
            } else {
                OnCeasePrimaryFire(*cWeapon, *activeWeapon);
            }
        }

        // Reload
        if (e.Command == "Reload" && e.Value != 0) {
            OnReload(*cWeapon, *activeWeapon);
        }

        return OnInputCommand(*cWeapon, *activeWeapon, e);
    }

    boost::optional<ComponentWrapper> getWeaponComponent(EntityWrapper player)
    {
        if (!player.HasComponent(m_ComponentType)) {
            return boost::none;
        }

        return player[m_ComponentType];
    }

    boost::optional<WeaponInfo&> getActiveWeapon(EntityWrapper player)
    {
        auto it = m_ActiveWeapons.find(player);
        if (it == m_ActiveWeapons.end()) {
            return boost::none;
        }
        WeaponInfo& activeWeapon = it->second;

        if (!activeWeapon.FirstPersonEntity.Valid() && !activeWeapon.ThirdPersonEntity.Valid()) {
            return boost::none;
        }

        return activeWeapon;
    }

    void selectWeapon(EntityWrapper player)
    {
        // Don't reselect weapon if it's already active
        if (getActiveWeapon(player)) {
            return;
        }

        // Find the weapon attachments matching the weapon type
        std::vector<EntityWrapper> weaponAttachments = player.ChildrenWithComponent("WeaponAttachment");
        EntityWrapper firstPersonAttachment;
        EntityWrapper thirdPersonAttachment;
        for (auto& attachment : weaponAttachments) {
            ComponentWrapper cWeaponAttachment = attachment["WeaponAttachment"];
            if ((std::string&)cWeaponAttachment["Weapon"] == m_ComponentType) {
                ComponentWrapper::SubscriptProxy person = cWeaponAttachment["Person"];
                if ((ComponentInfo::EnumType)person == person.Enum("FirstPerson")) {
                    firstPersonAttachment = attachment;
                } else if ((ComponentInfo::EnumType)person == person.Enum("ThirdPerson")) {
                    thirdPersonAttachment = attachment;
                }
            }
        }

        if (!firstPersonAttachment.Valid() && !thirdPersonAttachment.Valid()) {
            LOG_WARNING("No weapon attachment found for %s of player #%i", m_ComponentType.c_str(), player.ID);
            return;
        }

        // Spawn the weapon(s)
        EntityWrapper firstPersonWeapon;
        EntityWrapper thirdPersonWeapon;
        if (firstPersonAttachment.Valid()) {
            firstPersonWeapon = SpawnerSystem::Spawn(firstPersonAttachment, firstPersonAttachment);
        }
        if (thirdPersonAttachment.Valid()) {
            thirdPersonWeapon = SpawnerSystem::Spawn(thirdPersonAttachment, thirdPersonAttachment);
        }

        m_ActiveWeapons[player].Player = player;
        m_ActiveWeapons[player].WeaponEntity = player;
        m_ActiveWeapons[player].FirstPersonEntity = firstPersonWeapon;
        m_ActiveWeapons[player].ThirdPersonEntity = thirdPersonWeapon;
    }

    void holsterWeapon(ComponentWrapper cWeapon, EntityWrapper player)
    {
        auto activeWeapon = getActiveWeapon(player);
        if (!activeWeapon) {
            return;
        }
        WeaponInfo& wi = *activeWeapon;

        // Send holster event
        OnHolster(cWeapon, wi);

        // Delete weapon entities
        if (wi.FirstPersonEntity.Valid()) {
            m_World->DeleteEntity(wi.FirstPersonEntity.ID);
        }
        if (wi.ThirdPersonEntity.Valid()) {
            m_World->DeleteEntity(wi.ThirdPersonEntity.ID);
        }

        // Make weapon inactive
        m_ActiveWeapons.erase(player);
    }
};

#endif
