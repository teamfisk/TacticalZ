#include "Core/EntitySystem.h"
#include "Rendering/IRenderer.h"
#include "Core/Octree.h"
#include "Collision/EntityAABB.h"

class CapturePointsEntitySystem : public EntitySystem<World>
{
public:
    CapturePointsEntitySystem(EventBroker* eventBroker, bool isClient, bool isServer, IRenderer* renderer, RenderFrame* renderFrame);
    ~CapturePointsEntitySystem();

private:
    IRenderer* m_Renderer;
    RenderFrame* m_RenderFrame;
    Octree<EntityAABB>* m_OctreeCollision = nullptr;
    Octree<EntityAABB>* m_OctreeTrigger = nullptr;
    Octree<EntityAABB>* m_OctreeFrustrumCulling = nullptr;
};