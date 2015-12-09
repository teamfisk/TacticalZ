#include "OctTreeTestGameClass.h"

Game::Game(int argc, char* argv[]) : someOctTree(AABB(-0.5f*worldSize, 0.5f*worldSize), 2)
{
    ResourceManager::RegisterType<ConfigFile>("ConfigFile");
    ResourceManager::RegisterType<Model>("Model");
    ResourceManager::RegisterType<Texture>("Texture");

    m_Config = ResourceManager::Load<ConfigFile>("Config.ini");
    LOG_LEVEL = static_cast<_LOG_LEVEL>(m_Config->Get<int>("Debug.LogLevel", 1));

    // Create the core event broker
    m_EventBroker = new EventBroker();

    m_RenderQueueFactory = new RenderQueueFactory();

    // Create the renderer
    m_Renderer = new Renderer();
    m_Renderer->SetFullscreen(m_Config->Get<bool>("Video.Fullscreen", false));
    m_Renderer->SetVSYNC(m_Config->Get<bool>("Video.VSYNC", false));
    m_Renderer->SetResolution(Rectangle(
        0,
        0,
        m_Config->Get<int>("Video.Width", 1280),
        m_Config->Get<int>("Video.Height", 720)
        ));
    m_Renderer->Initialize();

    // Create input manager
    m_InputManager = new InputManager(m_Renderer->Window(), m_EventBroker);

    // Create the root level GUI frame
    m_FrameStack = new GUI::Frame(m_EventBroker);
    m_FrameStack->Width = m_Renderer->Resolution().Width;
    m_FrameStack->Height = m_Renderer->Resolution().Height;

    // Create a TEST WORLD
    m_World = new HardcodedTestWorld();

    m_LastTime = glfwGetTime();
}

Game::~Game()
{
    delete m_FrameStack;
    delete m_EventBroker;
}

void Game::Tick()
{
    double currentTime = glfwGetTime();
    double dt = currentTime - m_LastTime;
    m_LastTime = currentTime;

    m_EventBroker->Swap();
    m_InputManager->Update(dt);
    m_Renderer->Update(dt);
    m_EventBroker->Swap();

#define TEST2
#ifdef TEST1
    //add/move the trigger box
    auto pos = m_Renderer->Camera()->Forward() + m_Renderer->Camera()->Position();
    AABB boxi;
    boxi.CreateFromCenter(pos, maxPos - minPos);
    frameCounter++;
    if (frameCounter > 50) {
        m_World->someOctTree.ClearDynamicObjects();
        m_World->someOctTree.AddDynamicObject(boxi);
        frameCounter = 0;
    }
    ComponentWrapper transform = m_World->GetComponent(m_World->anotherBoxTransformId, "Transform");
    transform["Position"] = boxi.Center();

    //check all children again in the tree if they have a box in them or not, and colormark them if they do
    //contentboxarna får man ut - inte childboxarna!
    std::vector<int> boxIndex;
    boxIndex = m_World->someOctTree.childIndicesContainingBox(boxi);

    for (auto& oneLinkedObject : m_World->linkOM)
    {
        ComponentWrapper model = m_World->GetComponent(oneLinkedObject.entId, "Model");
        model["Color"] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        if (oneLinkedObject.child->m_DynamicObjects.size() != 0) {
            model["Color"] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        }

        //next check if the childIndicesContainingBox method returns the correct boxes
        //REQUIRED: childIndicesContainingBox must be public to test this!
        for each (auto someBoxIndex in boxIndex)
        {
            glm::vec3 pos = m_World->someOctTree.m_Children[someBoxIndex]->m_Box.Center();
            if (abs(pos.x - oneLinkedObject.posxyz.x) < 0.005f &&
                abs(pos.y - oneLinkedObject.posxyz.y) < 0.005f &&
                abs(pos.z - oneLinkedObject.posxyz.z) < 0.005f) {
                model["Color"] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

            }
        }
    }
    m_RenderQueueFactory->Update(m_World);

    //wireframe
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
#ifdef TEST2

    //only add 1 for now...
    //grey box
    AABB aabb;
    aabb.CreateFromCenter(glm::vec3(0.f, 2.f, 0.f), glm::vec3(1.f, 1.f, 1.f));

    const glm::vec4 redCol = glm::vec4(1, 0.2f, 0, 1);
    const glm::vec4 greenCol = glm::vec4(0.1f, 1.0f, 0.25f, 1);
    const glm::vec3 boxSize = 0.1f*glm::vec3(1.0f, 1.0f, 1.0f);

    if (!m_UpdatedOnce) {
        someOctTree.AddStaticObject(aabb);
        m_BoxID = m_World->CreateEntity();
        ComponentWrapper transform = m_World->AttachComponent(m_BoxID, "Transform");
        transform["Scale"] = boxSize;
        ComponentWrapper model = m_World->AttachComponent(m_BoxID, "Model");
        model["Resource"] = "Models/Core/UnitBox.obj";
        m_UpdatedOnce = true;
        m_World->createTestEntitiesTest2();
    }

    //red box
    AABB redBox;
    auto boxPos = m_Renderer->Camera()->Position() + 1.2f*m_Renderer->Camera()->Forward();
    redBox.CreateFromCenter(boxPos, boxSize);
    ComponentWrapper transform = m_World->GetComponent(m_BoxID, "Transform");
    transform["Position"] = boxPos;
    ComponentWrapper model = m_World->GetComponent(m_BoxID, "Model");
    if (someOctTree.BoxCollides(redBox, AABB())) {
    //if (Collision::AABBVsAABB(redBox, aabb)) {
        m_Renderer->Camera()->SetPosition(m_PrevPos);
        m_Renderer->Camera()->SetOrientation(m_PrevOri);
        model["Color"] = greenCol;
    }
    else {
        model["Color"] = redCol;
    }

    m_PrevPos = m_Renderer->Camera()->Position();
    m_PrevOri = m_Renderer->Camera()->Orientation();
    someOctTree.ClearObjects();

    m_RenderQueueFactory->Update(m_World);
#endif

    m_Renderer->Draw(m_RenderQueueFactory->RenderQueues());

    m_EventBroker->Swap();
    m_EventBroker->Clear();

    glfwPollEvents();
}
