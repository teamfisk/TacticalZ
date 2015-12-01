#include "Game.h"

Game::Game(int argc, char* argv[])
{
	ResourceManager::RegisterType<ConfigFile>("ConfigFile");

	m_Config = ResourceManager::Load<ConfigFile>("Config.ini");
	LOG_LEVEL = static_cast<_LOG_LEVEL>(m_Config->Get<int>("Debug.LogLevel", 1));

	// Create the core event broker
	m_EventBroker = new EventBroker();

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
	m_EventBroker->Swap();

	RenderQueueCollection rq;
	m_Renderer->Draw(rq);

	m_EventBroker->Swap();
	m_EventBroker->Clear();

	glfwPollEvents();
}
