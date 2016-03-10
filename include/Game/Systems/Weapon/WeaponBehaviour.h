#ifndef WeaponBehaviour_h__
#define WeaponBehaviour_h__

#include "Core/System.h"
#include "Rendering/IRenderer.h"
#include "Core/Octree.h"
#include "Collision/EntityAABB.h"
#include "Input/EInputCommand.h"
#include "Systems/SpawnerSystem.h"
#include "Rendering/ESetCamera.h"
#include "Core/ConfigFile.h"
#include "Rendering/EAutoAnimationBlend.h"

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
        EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &WeaponBehaviour::_OnInputCommand);
        EVENT_SUBSCRIBE_MEMBER(m_ESetCamera, &WeaponBehaviour::_OnSetCamera);
        auto config = ResourceManager::Load<ConfigFile>("Config.ini");
        m_ConfigAutoReload = config->Get<bool>("Gameplay.AutoReload", true);
    }
    virtual ~WeaponBehaviour() = default;

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cWeapon, double dt) override
    {
        /* EntityWrapper firstPersonWeapon = entity.FirstChildByName("Hands").FirstChildByName("AssaultWeapon");
         EntityWrapper thirdPersonWeapon = entity.FirstChildByName("PlayerModel").FirstChildByName("AssaultWeapon");
         if (IsClient && (firstPersonWeapon.Valid() || thirdPersonWeapon.Valid())) {
             if (m_ActiveWeapons.count(entity) == 0) {
                 WeaponInfo& wi = m_ActiveWeapons[entity];
                 wi.Player = entity;
                 wi.WeaponEntity = entity;
                 wi.FirstPersonEntity = firstPersonWeapon;
                 wi.ThirdPersonEntity = thirdPersonWeapon;

                 OnEquip(cWeapon, wi);
             }
         }*/

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
        EntityWrapper FirstPersonPlayerModel;
        EntityWrapper ThirdPersonEntity;
        EntityWrapper ThirdPersonPlayerModel;
    };

    IRenderer* m_Renderer;
    EntityWrapper m_CurrentCamera;
    Octree<EntityAABB>* m_CollisionOctree;
    std::unordered_map<EntityWrapper, WeaponInfo> m_ActiveWeapons;
    bool m_ConfigAutoReload;

    virtual void UpdateWeapon(ComponentWrapper cWeapon, WeaponInfo& wi, double dt) { }
    virtual void OnPrimaryFire(ComponentWrapper cWeapon, WeaponInfo& wi) { }
    virtual void OnCeasePrimaryFire(ComponentWrapper cWeapon, WeaponInfo& wi) { }
    virtual void OnReload(ComponentWrapper cWeapon, WeaponInfo& wi) { }
    virtual void OnEquip(ComponentWrapper cWeapon, WeaponInfo& wi) { }
    virtual void OnHolster(ComponentWrapper cWeapon, WeaponInfo& wi) { }
    virtual bool OnInputCommand(ComponentWrapper cWeapon, WeaponInfo& wi, const Events::InputCommand& e) { return false; }

    bool isPlayerInFirstPerson(EntityWrapper player)
    {
        if (!m_CurrentCamera.Valid()) {
            return false;
        } else {
            return m_CurrentCamera == player || m_CurrentCamera.IsChildOf(player);
        }
    }

    // Returns wi.FirstPersonEntity or wi.ThirdPersonEntity depending on
    // if the player is in first person mode or not.
    EntityWrapper getRelevantWeaponEntity(WeaponInfo& wi)
    {
        if (isPlayerInFirstPerson(wi.Player)) {
            return wi.FirstPersonEntity;
        } else {
            return wi.ThirdPersonEntity;
        }
    }

    float traceRayDistance(glm::vec3 origin, glm::vec3 direction)
    {
        float distance;
        glm::vec3 pos;
        auto entity = Collision::EntityFirstHitByRay(Ray(origin, direction), m_CollisionOctree, distance, pos);
        if (entity) {
            return distance;
        } else {
            return 100.f;
        }
    }

    void playAnimation(EntityWrapper weaponModelEntity, const std::string& subTreeName, const std::string& animationNodeName)
    {
        EntityWrapper root = weaponModelEntity.FirstParentWithComponent("Model");
        if (!root.Valid()) {
            return;
        }

        EntityWrapper subTree = root.FirstChildByName(subTreeName);
        if (!subTree.Valid()) {
            return;
        }

        EntityWrapper animationNode = subTree.FirstChildByName(animationNodeName);
        if (!animationNode.Valid()) {
            return;
        }

        Events::AutoAnimationBlend eFireBlend;
        eFireBlend.RootNode = root;
        eFireBlend.NodeName = animationNodeName;
        eFireBlend.Restart = true;
        eFireBlend.Start = true;
        m_EventBroker->Publish(eFireBlend);
    }

    void playAnimationAndReturn(EntityWrapper weaponModelEntity, const std::string& subTreeName, const std::string& animationNodeName)
    {
        EntityWrapper root = weaponModelEntity;
        if (!root.Valid()) {
            return;
        }

        EntityWrapper subTree = root.FirstChildByName(subTreeName);
        if (!subTree.Valid()) {
            return;
        }

        EntityWrapper animationNode = subTree.FirstChildByName(animationNodeName);
        if (!animationNode.Valid()) {
            return;
        }

        Events::AutoAnimationBlend eFireBlend;
        eFireBlend.RootNode = root;
        eFireBlend.NodeName = animationNodeName;
        eFireBlend.Restart = true;
        eFireBlend.Start = true;
        m_EventBroker->Publish(eFireBlend);

        Events::AutoAnimationBlend eIdleBlend;
        eIdleBlend.RootNode = root;
        eIdleBlend.NodeName = "Idle";
        eIdleBlend.AnimationEntity = animationNode;
        eIdleBlend.Delay = -0.2;
        eIdleBlend.Duration = 0.2;
        m_EventBroker->Publish(eIdleBlend);
    }

private:
    EventRelay<ETYPE, Events::SetCamera> m_ESetCamera;
    bool _OnSetCamera(const Events::SetCamera& e)
    {
        m_CurrentCamera = e.CameraEntity;
        return true;
    }
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
                    selectWeapon(*cWeapon, player);
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

    void selectWeapon(ComponentWrapper cWeapon, EntityWrapper player)
    {
        //if (!IsServer) {
        //    return;
        //}

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
        if (IsClient) {
            if (firstPersonAttachment.Valid()) {
                firstPersonWeapon = SpawnerSystem::Spawn(firstPersonAttachment, firstPersonAttachment);
            }
        }
        if (thirdPersonAttachment.Valid()) {
            thirdPersonWeapon = SpawnerSystem::Spawn(thirdPersonAttachment, thirdPersonAttachment);
        }

        WeaponInfo& wi = m_ActiveWeapons[player];
        wi.Player = player;
        wi.WeaponEntity = player;
        wi.FirstPersonEntity = firstPersonWeapon;
        wi.FirstPersonPlayerModel = firstPersonWeapon;
        wi.ThirdPersonEntity = thirdPersonWeapon;
        wi.ThirdPersonPlayerModel = thirdPersonWeapon.FirstParentWithComponent("Model");
        
        OnEquip(cWeapon, wi);
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
