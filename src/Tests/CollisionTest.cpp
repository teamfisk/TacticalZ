#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;
#include <Engine\Core\Collision.h>

BOOST_AUTO_TEST_SUITE(collisionTests)

BOOST_AUTO_TEST_CASE(collisionTest)
{
    Collision::Ray ray;
    ray.Origin = glm::vec3(0, 0, 0);
    ray.Direction = glm::vec3(0, 0, 0);
    auto someAABB = Collision::AABB(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
    bool z = Collision::RayVsAABB(ray, someAABB);
}

BOOST_AUTO_TEST_SUITE_END()

