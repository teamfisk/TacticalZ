//#define BOOST_TEST_MODULE collTest
#include <boost/test/unit_test.hpp>
#include <boost/test/execution_monitor.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;
#include "Engine/Collision/Collision.h"
#include "Engine/Core/AABB.h"
#include "Engine/Core/Ray.h"
#include <stdlib.h>//srand
#include "Engine/Core/OctTree.h"

//vs memleaks
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
//#define DEBUG_CLIENTBLOCK new( _CLIENT_BLOCK, __FILE__, __LINE__)
//#define new DEBUG_CLIENTBLOCK

BOOST_AUTO_TEST_SUITE(cTest)
BOOST_AUTO_TEST_SUITE_END()

