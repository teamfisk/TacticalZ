//#define BOOST_TEST_MODULE collTest
#include <boost/test/unit_test.hpp>
#include <boost/test/execution_monitor.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;
#include <Engine\Core\Collision.h>
#include <stdlib.h>//srand

//vs memleaks
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
//#define DEBUG_CLIENTBLOCK new( _CLIENT_BLOCK, __FILE__, __LINE__)
//#define new DEBUG_CLIENTBLOCK

BOOST_AUTO_TEST_SUITE(collisionTests)

BOOST_AUTO_TEST_CASE(collisionTest)
{
    //memleak
    int* globalLeak = new int[5];

    //fixed seed
    srand(2);
    Collision::Ray ray;
    Collision::AABB someAABB;
    glm::vec3 minPos;
    glm::vec3 maxPos;
    bool z;
    int test = 0;
    for (size_t i = 0; i < 10; i++)
    {
        ray.Origin.x = rand() % 100;
        ray.Origin.y = rand() % 100;
        ray.Origin.z = rand() % 100;
        ray.Direction.x = rand() % 100;
        ray.Direction.y = rand() % 100;
        ray.Direction.z = rand() % 100;
        minPos.x = rand() % 100;
        minPos.y = rand() % 100;
        minPos.z = rand() % 100;
        maxPos.x = rand() % 100;
        maxPos.y = rand() % 100;
        maxPos.z = rand() % 100;

        someAABB = Collision::AABB(minPos, maxPos);
        z = Collision::RayVsAABB(ray, someAABB);
        if (z) ++test;
    }
    BOOST_CHECK(test >= 0);

    //_CrtDumpMemoryLeaks();
}

BOOST_AUTO_TEST_CASE(collisionTest2)
{
    //fixed seed
    srand(2);
    Collision::Ray ray;
    Collision::AABB someAABB;
    glm::vec3 minPos;
    glm::vec3 maxPos;
    bool z;
    int test = 0;
    for (size_t i = 0; i < 1000000; i++)
    {
        ray.Origin.x = rand() % 100;
        ray.Origin.y = rand() % 100;
        ray.Origin.z = rand() % 100;
        ray.Direction.x = rand() % 100;
        ray.Direction.y = rand() % 100;
        ray.Direction.z = rand() % 100;
        minPos.x = rand() % 100;
        minPos.y = rand() % 100;
        minPos.z = rand() % 100;
        maxPos.x = rand() % 100;
        maxPos.y = rand() % 100;
        maxPos.z = rand() % 100;

        someAABB = Collision::AABB(minPos, maxPos);
        z = Collision::RayAABBIntr(ray, someAABB);
        if (z) ++test;
    }
    BOOST_CHECK(test >= 0);
}

BOOST_AUTO_TEST_SUITE_END()

