#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "configdir.hpp"
#include "auth.hpp"
using namespace Crowd;

BOOST_AUTO_TEST_SUITE(AuthTest)

Auth a;

BOOST_AUTO_TEST_CASE(SetNetwork)
{
    BOOST_CHECK(a.setNetwork("Default") == 0);
    BOOST_CHECK(a.setNetwork("error") == 1);
}
BOOST_AUTO_TEST_CASE(ValidateEmail)
{
    std::string email1 = "test@test.com";
    std::string email2 = "testtest.com";
    BOOST_CHECK(a.validateEmail(email1) == 0);
    BOOST_CHECK(a.validateEmail(email2) == 1);
}
BOOST_AUTO_TEST_CASE(VerifyCredentials)
{
    // first a new user must be created, which is not yet implemented
    //BOOST_CHECK(a.verifyCredentials("test@test.com", "testpass") == "0"); 
}

BOOST_AUTO_TEST_SUITE_END()