#ifndef Game_h__
#define Game_h__

#include <boost/program_options.hpp>

#include "Core/ResourceManager.h"
#include "Core/ConfigFile.h"
#include "Core/EventBroker.h"
#include "Rendering/Renderer.h"
#include "Core/InputManager.h"
#include "Core/World.h"
#include "Input/InputProxy.h"
#include "Input/KeyboardInputHandler.h"
#include "Input/MouseInputHandler.h"
#include "Core/EKeyDown.h"
#include "Core/EntityXMLFilePreprocessor.h"
#include "Core/SystemPipeline.h"
#include "Systems/ExplosionEffectSystem.h"
#include "Editor/EditorSystem.h"
#include "Core/EntityXMLFile.h"
#include "Rendering/RenderSystem.h"
#include "Core/EntityXMLFileParser.h"
#include "Core/Octree.h"
#include "Rendering/Font.h"
#include "Systems/InterpolationSystem.h"
#include "Collision/EntityAABB.h"
// Network
#include <boost/thread.hpp>
#include "Network/Network.h"
#include "Network/Server.h"
#include "Network/Client.h"

// Sound
#include "Sound/SoundManager.h"
#include "Systems/SoundSystem.h"

//Performance 
#include "Core/PerformanceTimer.h"

class Game
{
public:
	Game(int argc, char* argv[]);
	~Game();
	
	bool Running() const { return !glfwWindowShouldClose(m_Renderer->Window()); }
	void Tick();

private:
    std::string m_NetworkAddress;
    int m_NetworkPort = 0;

	ConfigFile* m_Config = nullptr;
	EventBroker* m_EventBroker;
	IRenderer* m_Renderer;
	InputManager* m_InputManager;
    InputProxy* m_InputProxy;
    World* m_World;
    Octree<EntityAABB>* m_OctreeCollision;
    Octree<EntityAABB>* m_OctreeTrigger;
    Octree<EntityAABB>* m_OctreeFrustrumCulling;
    SystemPipeline* m_SystemPipeline;
    RenderFrame* m_RenderFrame;
    Client* m_NetworkClient = nullptr;
    Server* m_NetworkServer = nullptr;
    SoundManager* m_SoundManager;
	double m_LastTime;

    bool m_IsClient = false;
    bool m_IsServer = false;

    int parseArgs(int argc, char* argv[]);
};

#endif
