#include "Game.h"
#include "Collision/TriggerSystem.h"
#include "Collision/CollisionSystem.h"

Game::Game(int argc, char* argv[])
{
    ResourceManager::RegisterType<ConfigFile>("ConfigFile");
    ResourceManager::RegisterType<Model>("Model");
    ResourceManager::RegisterType<Texture>("Texture");
    ResourceManager::RegisterType<EntityXMLFile>("EntityXMLFile");
    ResourceManager::RegisterType<ShaderProgram>("ShaderProgram");

    m_Config = ResourceManager::Load<ConfigFile>("Config.ini");
    LOG_LEVEL = static_cast<_LOG_LEVEL>(m_Config->Get<int>("Debug.LogLevel", 1));

    // Create the core event broker
    m_EventBroker = new EventBroker();

    m_RenderQueueFactory = new RenderQueueFactory();

    // Create the renderer
    m_Renderer = new Renderer(m_EventBroker);
    m_Renderer->SetFullscreen(m_Config->Get<bool>("Video.Fullscreen", false));
    m_Renderer->SetVSYNC(m_Config->Get<bool>("Video.VSYNC", false));
    m_Renderer->SetResolution(Rectangle::Rectangle(
        0,
        0,
        m_Config->Get<int>("Video.Width", 1280),
        m_Config->Get<int>("Video.Height", 720)
        ));
    m_Renderer->Initialize();
    m_Renderer->Camera()->SetFOV(glm::radians(m_Config->Get<float>("Video.FOV", 90.f)));

    // Create input manager
    m_InputManager = new InputManager(m_Renderer->Window(), m_EventBroker);
    m_InputProxy = new InputProxy(m_EventBroker);
    m_InputProxy->AddHandler<KeyboardInputHandler>();
    m_InputProxy->AddHandler<MouseInputHandler>();
    m_InputProxy->LoadBindings("Input.ini");

    // Create the root level GUI frame
    m_FrameStack = new GUI::Frame(m_EventBroker);
    m_FrameStack->Width = m_Renderer->Resolution().Width;
    m_FrameStack->Height = m_Renderer->Resolution().Height;

    // Create a world
    m_World = new World();
    std::string mapToLoad = m_Config->Get<std::string>("Debug.LoadMap", "");
    if (!mapToLoad.empty()) {
        ResourceManager::Load<EntityXMLFile>(mapToLoad)->PopulateWorld(m_World);
    }

    // Create system pipeline
    m_SystemPipeline = new SystemPipeline(m_EventBroker);
    m_SystemPipeline->AddSystem<RaptorCopterSystem>();
    m_SystemPipeline->AddSystem<PlayerSystem>();
    m_SystemPipeline->AddSystem<EditorSystem>(m_Renderer);
    m_SystemPipeline->AddSystem<CollisionSystem>();
    m_SystemPipeline->AddSystem<TriggerSystem>();
    m_SystemPipeline->AddSystem<CoolDeathAnimationSystem>();

    // Invoke network
    if (m_Config->Get<bool>("Networking.StartNetwork", false)) {
        //boost::thread workerThread(&Game::networkFunction, this);
        networkFunction();
    }
    m_LastTime = glfwGetTime();

    debugInitialize();
}

Game::~Game()
{
    // Call before to ensure that thread closes correctly.
    //if (m_IsClientOrServer)
    //  m_ClientOrServer.Close();

    delete m_FrameStack;
    delete m_EventBroker;
}

void Game::Tick()
{
    glfwPollEvents();

    double currentTime = glfwGetTime();
    double dt = currentTime - m_LastTime;
    m_LastTime = currentTime;

    // Handle input in a weird looking but responsive way
    m_EventBroker->Process<InputManager>();
    m_EventBroker->Swap();
    m_InputManager->Update(dt);
    m_EventBroker->Swap();
    m_InputProxy->Update(dt);
    m_EventBroker->Swap();
    m_InputProxy->Process();
    m_EventBroker->Swap();

    // Update network
    if (m_IsClientOrServer) {
        m_ClientOrServer->Update();
    }

    // Iterate through systems and update world!
    m_SystemPipeline->Update(m_World, dt);
    debugTick(dt);
    m_Renderer->Update(dt);
    m_EventBroker->Process<Client>();

    m_RenderQueueFactory->Update(m_World);
    GLERROR("Game::Tick m_RenderQueueFactory->Update");
    m_Renderer->Draw(m_RenderQueueFactory->RenderQueues());
    GLERROR("Game::Tick m_Renderer->Draw");
    m_EventBroker->Swap();
    m_EventBroker->Clear();
}


bool Game::debugOnInputCommand(const Events::InputCommand& e)
{
    if (e.Command == "DebugReload" && e.Value == 1) {
        std::string mapToLoad = m_Config->Get<std::string>("Debug.LoadMap", "");
        if (!mapToLoad.empty()) {
            delete m_World;
            m_World = new World();
            ResourceManager::Release("EntityXMLFile", mapToLoad);
            ResourceManager::Load<EntityXMLFile>(mapToLoad)->PopulateWorld(m_World);
        }
    }
    if (e.Command == "SwitchToServer" && e.Value > 0) {
        m_ClientOrServer = new Server();
        LOG_INFO("Switching to server");
        m_ClientOrServer->Start(m_World, m_EventBroker);
    }
    if (e.Command == "SwitchToClient" && e.Value > 0) {
        m_ClientOrServer = new Client(m_Config);
        m_ClientOrServer->Start(m_World, m_EventBroker);
        LOG_INFO("Switching to client");
    }

    return false;
}

void Game::debugInitialize()
{
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &Game::debugOnInputCommand);
}

void Game::debugTick(double dt)
{
    m_EventBroker->Process<Game>();
}

void Game::networkFunction()
{
    bool isServer = m_Config->Get<bool>("Networking.IsServer", false);
    if (!isServer) {
        m_IsClientOrServer = true;
        m_ClientOrServer = new Client(m_Config);
    }
    if (isServer) {
        m_IsClientOrServer = true;
        m_ClientOrServer = new Server();
    }
    m_ClientOrServer->Start(m_World, m_EventBroker);
    // I don't think we are reaching this part of the code right now.
    // ~Game() is not called if the game is exited by closing console windows
    // When server or client is done set it to false.
    //m_IsClientOrServer = false;
    // Destroy it
    //delete m_ClientOrServer;
}