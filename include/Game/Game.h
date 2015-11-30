#ifndef Game_h__
#define Game_h__

#include "Core/ResourceManager.h"
#include "Core/ConfigFile.h"

#include "Core/EventBroker.h"
#include "Rendering/DummyRenderer.h"
#include "Core/InputManager.h"

#include "GUI/Frame.h"

class Game
{
public:
	Game(int argc, char* argv[]);
	~Game();
	
	bool Running() const { return true; }
	void Tick();

private:
	double m_LastTime;
	ConfigFile* m_Config = nullptr;
	EventBroker* m_EventBroker;
	IRenderer* m_Renderer;
	InputManager* m_InputManager;
	GUI::Frame* m_FrameStack;
};

#endif