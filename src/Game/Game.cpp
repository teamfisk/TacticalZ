#include "Game.h"

Game::Game(int argc, char* argv[])
{
	ResourceManager::RegisterType<ConfigFile>("ConfigFile");
	ResourceManager::RegisterType<Model>("Model");
	ResourceManager::RegisterType<Texture>("Texture");
    ResourceManager::RegisterType<EntityXMLFile>("EntityXMLFile");

	m_Config = ResourceManager::Load<ConfigFile>("Config.ini");
	LOG_LEVEL = static_cast<_LOG_LEVEL>(m_Config->Get<int>("Debug.LogLevel", 1));

	// Create the core event broker
	m_EventBroker = new EventBroker();

    m_RenderQueueFactory = new RenderQueueFactory();

	// Create the renderer
	m_Renderer = new Renderer(m_EventBroker);
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



	m_LastTime = glfwGetTime();

    testIntialize();
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
	m_EventBroker->Swap();

    // Iterate through systems and update world!
    m_SystemPipeline->Update(m_World, dt);
    testTick(dt);
    m_Renderer->Update(dt);

    m_RenderQueueFactory->Update(m_World);
	m_Renderer->Draw(m_RenderQueueFactory->RenderQueues());

	m_EventBroker->Swap();
	m_EventBroker->Clear();

	glfwPollEvents();
}


bool Game::testOnKeyUp(const Events::KeyUp& e)
{
    if (e.KeyCode == GLFW_KEY_R) {
        std::string mapToLoad = m_Config->Get<std::string>("Debug.LoadMap", "");
        if (!mapToLoad.empty()) {
            delete m_World;
            m_World = new World();
            ResourceManager::Release("EntityXMLFile", mapToLoad);
            ResourceManager::Load<EntityXMLFile>(mapToLoad)->PopulateWorld(m_World);
        }
    }

    return false;
}

void Game::testIntialize()
{
    EVENT_SUBSCRIBE_MEMBER(m_EKeyUp, &Game::testOnKeyUp);
}

void Game::testTick(double dt)
{
    m_EventBroker->Process<Game>();
}
