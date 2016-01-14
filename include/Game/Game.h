#ifndef Game_h__
#define Game_h__

#include "Core/ResourceManager.h"
#include "Core/ConfigFile.h"
#include "Core/EventBroker.h"
#include "Rendering/Renderer.h"
#include "Core/InputManager.h"
#include "GUI/Frame.h"
#include "Core/World.h"
#include "Rendering/RenderQueueFactory.h"
#include "Input/InputProxy.h"
#include "Input/KeyboardInputHandler.h"
#include "Input/MouseInputHandler.h"
#include "Core/EKeyDown.h"
#include "Core/EntityFilePreprocessor.h"
#include "Core/SystemPipeline.h"
#include "RaptorCopterSystem.h"
#include "PlayerSystem.h"
#include "Editor/EditorSystem.h"
#include "Core/EntityFile.h"
#include "Core/EntityFileParser.h"

// Network
#include <boost/thread.hpp>
#include "Network/Network.h"
#include "Network/Server.h"
#include "Network/Client.h"

// Sound
#include "Sound/SoundSystem.h"

class Game
{
public:
	Game(int argc, char* argv[]);
	~Game();
	
	bool Running() const { return !glfwWindowShouldClose(m_Renderer->Window()); }
	void Tick();

private:
	double m_LastTime;
	ConfigFile* m_Config = nullptr;
	EventBroker* m_EventBroker;
	IRenderer* m_Renderer;
	InputManager* m_InputManager;
    InputProxy* m_InputProxy;
	GUI::Frame* m_FrameStack;
    World* m_World;
    SystemPipeline* m_SystemPipeline;
    RenderQueueFactory* m_RenderQueueFactory;
    // Network variables
    boost::thread m_NetworkThread;

    // Network methods
    void networkFunction();
    Network* m_ClientOrServer;
    bool m_IsClientOrServer = false;

    // Sound
    SoundSystem* m_SoundSystem;

    //EventRelay<Game, Events::InputCommand> m_EInputCommand;
    //bool debugOnInputCommand(const Events::InputCommand& e);

    void debugInitialize();
    void debugTick(double dt);
	EventRelay<Client, Events::KeyDown> m_EKeyDown;

};

#endif
