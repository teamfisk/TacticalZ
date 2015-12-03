#ifndef Game_h__
#define Game_h__

#include "Core/ResourceManager.h"
#include "Core/ConfigFile.h"
#include "Core/EventBroker.h"
#include "Rendering/Renderer.h"
#include "Core/InputManager.h"
#include "GUI/Frame.h"
#include "Core/World.h"

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
	GUI::Frame* m_FrameStack;
    World* m_World;
};

#endif
