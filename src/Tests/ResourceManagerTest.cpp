#include <boost/test/unit_test.hpp>

#include "Core/World.h"

//#define private public
#include "Core/ResourceManager.h"

#include "Core/ConfigFile.h"

#include "Rendering/Renderer.h"
#include "Core/EntityXMLFile.h"
#include "Engine\Rendering\Texture.h"

//#include "Core/EventBroker.h"
//#include "Core/InputManager.h"
//#include "GUI/Frame.h"
//#include "Rendering/RenderQueueFactory.h"
//#include "Core/EKeyDown.h"
//#include "Core/SystemPipeline.h"
//#include "RaptorCopterSystem.h"


BOOST_AUTO_TEST_SUITE(resourceManagerTests)

BOOST_AUTO_TEST_CASE(resourceManagerTest)
{
    World m_World;

    //private static metoder/variabler

    //ugly private->public hack doesnt work, tons of link errors. hence cant test it properly
    //its not my job to implement testfunctions for unittests in the class either

    //craptests ahead:
    ResourceManager::RegisterType<ConfigFile>("ConfigFile");
    BOOST_CHECK(!ResourceManager::IsResourceLoaded("ConfigFile", "Config.ini"));
    auto m_Config = ResourceManager::Load<ConfigFile>("Config.ini");
    BOOST_CHECK(ResourceManager::IsResourceLoaded("ConfigFile", "Config.ini"));
    ResourceManager::Release("ConfigFile", "Config.ini");
    BOOST_CHECK(!ResourceManager::IsResourceLoaded("ConfigFile", "Config.ini"));

    //configfile without register
    //check so output says "EE failed to load: type not registered..."
    auto m_ScreenQuadNoRegister = ResourceManager::Load<Model>("Models/Core/ScreenQuad.obj");
    BOOST_CHECK(!ResourceManager::IsResourceLoaded("Model", "Models/Core/ScreenQuad.obj"));

    //there is no error feedback to check if you try to release the wrong resources - hence that cant be tested either

    //registertype (bind with function)
    //m_CompilerTypenameToResourceType = global...
    //m_FactoryFunctions = global...
    //BOOST_CHECK(ResourceManager::m_CompilerTypenameToResourceType.size() != 0);
    //BOOST_CHECK(ResourceManager::m_FactoryFunctions.size() != 0);
}

BOOST_AUTO_TEST_SUITE_END()
