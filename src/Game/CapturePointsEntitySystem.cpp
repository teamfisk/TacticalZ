#include "CapturePointsEntitySystem.h"
#include "Systems/InterpolationSystem.h"
#include "Systems/SoundSystem.h"
#include "Systems/RaptorCopterSystem.h"
#include "Systems/ExplosionEffectSystem.h"
#include "Systems/HealthSystem.h"
#include "Systems/PlayerMovementSystem.h"
#include "Systems/SpawnerSystem.h"
#include "Systems/PlayerSpawnSystem.h"
#include "Systems/Weapon/WeaponSystem.h"
#include "Systems/LifetimeSystem.h"
#include "Systems/CapturePointSystem.h"
#include "Systems/CapturePointHUDSystem.h"
#include "Systems/PickupSpawnSystem.h"
#include "Systems/AmmoPickupSystem.h"
#include "Systems/DamageIndicatorSystem.h"
#include "Systems/AmmunitionHUDSystem.h"
#include "Systems/KillFeedSystem.h"
#include "GUI/ButtonSystem.h"
#include "GUI/MainMenuSystem.h"
#include "Collision/FillOctreeSystem.h"
#include "Collision/FillFrustumOctreeSystem.h"
#include "Rendering/AnimationSystem.h"
#include "Core/UniformScaleSystem.h"
#include "Systems/HealthHUDSystem.h"
#include "Systems/PlayerDeathSystem.h"
#include "Rendering/BoneAttachmentSystem.h"
#include "Collision/CollisionSystem.h"
#include "Collision/TriggerSystem.h"
#include "Rendering/RenderSystem.h"
#include "Editor/EditorSystem.h"

CapturePointsEntitySystem::CapturePointsEntitySystem(EventBroker* eventBroker, bool isClient, bool isServer, IRenderer* renderer, RenderFrame* renderFrame)
    : EntitySystem(eventBroker, isClient, isServer)
    , m_Renderer(renderer)
    , m_RenderFrame(renderFrame)
{
    // Create Octrees
    // TODO: Perhaps the world bounds should be set in some non-arbitrary way instead of this.
    AABB boxContainingTheWorld(glm::vec3(-300), glm::vec3(300));
    m_OctreeCollision = new Octree<EntityAABB>(boxContainingTheWorld, 4);
    m_OctreeTrigger = new Octree<EntityAABB>(boxContainingTheWorld, 4);
    m_OctreeFrustrumCulling = new Octree<EntityAABB>(boxContainingTheWorld, 4);
    
    // All systems with orderlevel 0 will be updated first.
    unsigned int updateOrderLevel = 0;
    AddSystem<InterpolationSystem>(updateOrderLevel);
    ++updateOrderLevel;
    AddSystem<SoundSystem>(updateOrderLevel);
    AddSystem<RaptorCopterSystem>(updateOrderLevel);
    AddSystem<ExplosionEffectSystem>(updateOrderLevel);
    AddSystem<HealthSystem>(updateOrderLevel);
    AddSystem<PlayerMovementSystem>(updateOrderLevel);
    AddSystem<SpawnerSystem>(updateOrderLevel);
    AddSystem<PlayerSpawnSystem>(updateOrderLevel);
    AddSystem<WeaponSystem>(updateOrderLevel, m_Renderer, m_OctreeCollision);
    AddSystem<LifetimeSystem>(updateOrderLevel);
    AddSystem<CapturePointSystem>(updateOrderLevel);
    AddSystem<CapturePointHUDSystem>(updateOrderLevel);
    AddSystem<PickupSpawnSystem>(updateOrderLevel);
    AddSystem<AmmoPickupSystem>(updateOrderLevel);
    AddSystem<DamageIndicatorSystem>(updateOrderLevel);
    AddSystem<AmmunitionHUDSystem>(updateOrderLevel);
    AddSystem<KillFeedSystem>(updateOrderLevel);
    AddSystem<ButtonSystem>(updateOrderLevel, m_Renderer);
    AddSystem<MainMenuSystem>(updateOrderLevel, m_Renderer);
    // Populate Octree with collidables
    ++updateOrderLevel;
    AddSystem<FillOctreeSystem>(updateOrderLevel, m_OctreeCollision, "Collidable");
    AddSystem<FillOctreeSystem>(updateOrderLevel, m_OctreeTrigger, "Player");
    AddSystem<FillFrustumOctreeSystem>(updateOrderLevel, m_OctreeFrustrumCulling);
    AddSystem<AnimationSystem>(updateOrderLevel);
    AddSystem<UniformScaleSystem>(updateOrderLevel);
    AddSystem<HealthHUDSystem>(updateOrderLevel);
    AddSystem<PlayerDeathSystem>(updateOrderLevel);
    // Collision and TriggerSystem should update after player.
    ++updateOrderLevel;
    AddSystem<BoneAttachmentSystem>(updateOrderLevel);
    AddSystem<CollisionSystem>(updateOrderLevel, m_OctreeCollision);
    AddSystem<TriggerSystem>(updateOrderLevel, m_OctreeTrigger);
    ++updateOrderLevel;
    AddSystem<RenderSystem>(updateOrderLevel, m_Renderer, m_RenderFrame, m_OctreeFrustrumCulling);
    ++updateOrderLevel;
    AddSystem<EditorSystem>(updateOrderLevel, m_Renderer, m_RenderFrame);
}

CapturePointsEntitySystem::~CapturePointsEntitySystem()
{
    if (m_OctreeFrustrumCulling != nullptr) {
        delete m_OctreeFrustrumCulling;
    }
    if (m_OctreeCollision != nullptr) {
        delete m_OctreeCollision;
    }
    if (m_OctreeTrigger != nullptr) {
        delete m_OctreeTrigger;
    }
}
