#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;
#include <stdlib.h>//srand
//HACK! Needed for white box testing
//else we would have to "open up" the octTree class more with get/sets, public methods, etc. which is not good encapsulation-wise
//friend class and refactoringIntoNewClass is some extra work and needs to be updated when the original class is updated, and can contain bugs that
//isnt in the original class
//Reflection-inspection seems to be only available for C#
//http://stackoverflow.com/questions/6778496/how-to-do-unit-testing-on-private-members-and-methods-of-c-classes
//http://stackoverflow.com/questions/3676664/unit-testing-of-private-methods
#define private public

#include <Engine\Core\OctTree.h>

BOOST_AUTO_TEST_SUITE(octTreeTests)

BOOST_AUTO_TEST_CASE(octTreeTest)
{
    //white box testing
    //http://softwaretestingfundamentals.com/differences-between-black-box-testing-and-white-box-testing/
    //http://technologyconversations.com/2013/12/11/black-box-vs-white-box-testing/

    //simple AABB constructor check
    auto minCorner = glm::vec3(0.0f, 0.0f, 0.0f);
    auto maxCorner = glm::vec3(1.0f, 1.0f, 1.0f);
    auto someAABB = AABB(minCorner,maxCorner);
    BOOST_CHECK(someAABB.MinCorner() == minCorner);
    BOOST_CHECK(someAABB.MaxCorner() == maxCorner);
    BOOST_CHECK(someAABB.Center() == 0.5f * (minCorner + maxCorner));

    //simple OctTree constructor check
    auto someOctTree = OctTree(someAABB, 5);
    BOOST_CHECK(someOctTree.m_Children[0] != nullptr);
    //TODO: a check so it split the tree properly




    //advanced AddBox check
    //add a boxcontainer - which crosses the mid-split
    auto someAABB2 = AABB(glm::vec3(0.45f, 0.45f, 0.45f), glm::vec3(0.55f, 0.55f, 0.55f));
    someOctTree.AddBox(someAABB2);
    //clear the boxcontainer
    //need to check so it added the box properly


    someOctTree.ClearBoxes();
    //add a boxcontainer
    someOctTree.AddBox(someAABB2);


    //simple destructor check in the end, just look for memleaks, then it didnt clear the AABB structure
}

BOOST_AUTO_TEST_CASE(octTreeTest2)
{

}

BOOST_AUTO_TEST_SUITE_END()
