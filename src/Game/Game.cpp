#include "Game.h"
#include "HardcodedTestWorld.h"

Game::Game(int argc, char* argv[])
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

    m_RenderQueueFactory->Update(m_World);

	m_Renderer->Draw(m_RenderQueueFactory->RenderQueues());

	m_EventBroker->Swap();
	m_EventBroker->Clear();

	glfwPollEvents();
}
