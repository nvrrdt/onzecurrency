/*#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "terminal/main.hpp"
using namespace Crowd;

BOOST_AUTO_TEST_SUITE(AuthTest)

Crowd::ConfigDir cd;

BOOST_AUTO_TEST_CASE(GetConfigDir)
{
    BOOST_CHECK(boost::algorithm::ends_with(cd.GetConfigDir(), ".config/onzehub/"));
}
BOOST_AUTO_TEST_CASE(CreateFileInConfigDir)
{
    BOOST_CHECK(cd.CreateFileInConfigDir("test.txt", "test") == 0);
}

BOOST_AUTO_TEST_SUITE_END()
*/