#include <boost/test/unit_test.hpp>

#include "poco.hpp"
using namespace Crowd;

BOOST_AUTO_TEST_SUITE(PocoTest)

Poco p;

BOOST_AUTO_TEST_CASE(Get)
{
    //BOOST_TEST_MESSAGE("Get: " << p.Get(1)); // ./build/tests --log_level=message
    BOOST_CHECK(p.Get(1) == "");
}

BOOST_AUTO_TEST_CASE(PutGetDelete)
{
    BOOST_CHECK(p.Put(2, "test2") == 0);
    BOOST_CHECK(p.Get(2) == "test2");
    BOOST_CHECK(p.Delete(2) == 0);
    //BOOST_TEST_MESSAGE("Delete: " << p.Get(2));
    BOOST_CHECK(p.Get(2) == "");
}

BOOST_AUTO_TEST_CASE(FindChosenOne)
{
    BOOST_CHECK(p.Put(2, "test2") == 0);
    BOOST_CHECK(p.FindChosenOne(1) == 2);
    BOOST_CHECK(p.FindChosenOne(2) == 2);
}

BOOST_AUTO_TEST_SUITE_END()