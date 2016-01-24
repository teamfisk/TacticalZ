#include <boost/test/unit_test.hpp>
#include "Core/World.h"

//private->public hack doesnt work, tons of link errors
//so there is currently no good way to test this class
//#define private public
#include "Core/ResourceManager.h"
#include "Core/ConfigFile.h"
#include "Rendering/Renderer.h"
#include "Engine/Rendering/Texture.h"

BOOST_AUTO_TEST_SUITE(resourceManagerTests)

BOOST_AUTO_TEST_CASE(resourceManagerTest)
{
    World m_World;

    //private static metoder/variabler

    ResourceManager::RegisterType<ConfigFile>("ConfigFile");
    BOOST_CHECK(!ResourceManager::IsResourceLoaded("ConfigFile", "Config.ini"));

    BOOST_CHECK_NO_THROW(ResourceManager::Load<ConfigFile>("Config.ini"));
    BOOST_CHECK(ResourceManager::IsResourceLoaded("ConfigFile", "Config.ini"));
    ResourceManager::Release("ConfigFile", "Config.ini");
    BOOST_CHECK(!ResourceManager::IsResourceLoaded("ConfigFile", "Config.ini"));

    //configfile without register
    BOOST_CHECK_THROW(ResourceManager::Load<Model>("Models/Core/ScreenQuad.obj"),Resource::FailedLoadingException);
    //there is no error feedback to check if you try to release the wrong resources - hence that cant be tested either
}

BOOST_AUTO_TEST_SUITE_END()
