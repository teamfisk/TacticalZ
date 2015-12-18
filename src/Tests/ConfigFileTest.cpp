#include <boost/test/unit_test.hpp>
#include <boost/test/execution_monitor.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;
#include <stdlib.h>//srand

//#define private public
#include "Engine\Core\ConfigFile.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define DEBUG_CLIENTBLOCK new( _CLIENT_BLOCK, __FILE__, __LINE__)
#define new DEBUG_CLIENTBLOCK

BOOST_AUTO_TEST_SUITE(confTest)

BOOST_AUTO_TEST_CASE(configFileTest)
{
    //note: this ConfigFileclass currently has memleaks!

    ResourceManager::RegisterType<ConfigFile>("ConfigFile");
    auto m_Config = ResourceManager::Load<ConfigFile>("ConfigTest.ini");

    //bägge måste vara av samma typ, T typen är string
    //http://www.boost.org/doc/libs/1_42_0/doc/html/boost_propertytree/tutorial.html
    //"Note that we construct the path to the value by separating the individual keys with dots"

    //get from tree tests
    auto getSomething = m_Config->Get("Test.Test1", 0);
    BOOST_CHECK(getSomething == 423);

    auto getSomething2 = m_Config->Get("fsdfdsfd.T", std::string(""));
    BOOST_CHECK(getSomething2 == "\"gfdjakflsdl!\"");

    //set/get tests
    m_Config->Set("Test.4321", 123);
    auto getSomething3 = m_Config->Get("Test.4321", 0);
    BOOST_CHECK(getSomething3 == 123);

    m_Config->Set("3_2_1_0_5", "t454j54hj5k32");
    auto getSomething4 = m_Config->Get("3_2_1_0_5", std::string(""));
    BOOST_CHECK(getSomething4 == "t454j54hj5k32");

    //***check so outputwindow says: EE: Failed to find "DefaultConfigTestNotExists.ini"! Relying on hardcoded default values!
    auto m_Config2 = ResourceManager::Load<ConfigFile>("ConfigTestNotExists.ini");

    //set value/savetodisk/load/checkvalue...
    m_Config->SaveToDisk();
    m_Config->Set("Test.4321", 145);
    m_Config->SaveToDisk();
    auto m_Config3 = ResourceManager::Load<ConfigFile>("ConfigTest.ini");
    auto getSomething5 = m_Config->Get("Test.4321", 0);
    BOOST_CHECK(getSomething5 == 145);
    
    //***check so outputwindow says: EE: Failed to parse "DefaultConfigTestFailed.ini"
    //***check so outputwindow says: EE: Failed to parse "ConfigTestFailed.ini":
    auto m_Config4 = ResourceManager::Load<ConfigFile>("ConfigTestFailed.ini");

    //reload,onchildreload unimplemented

    //NOTE:still massive amount of memoryleaks from this method
    _CrtDumpMemoryLeaks();
}

BOOST_AUTO_TEST_SUITE_END()

