#include <boost/test/unit_test.hpp>
#include "EventFixture.h"

struct ETestEvent : public Event
{
    int Int = 5;
    float Float = 1.33333f;
    double Double = 1.33333;
    std::string String = "Hello World";
};

BOOST_AUTO_TEST_CASE(EventBrokerTest)
{
    EventFixture<ETestEvent> f;
    BOOST_CHECK(f.Before.Int == f.After.Int);
    BOOST_CHECK_CLOSE(f.Before.Float, f.After.Float, 0.00001f);
    BOOST_CHECK_CLOSE(f.Before.Double, f.After.Double, 0.00001f);
    BOOST_CHECK(f.Before.String == f.After.String);
}