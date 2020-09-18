#include <boost/test/unit_test.hpp>

#include "hash.hpp"
using namespace Crowd;

BOOST_AUTO_TEST_SUITE(HashTest)

Hash h;

BOOST_AUTO_TEST_CASE(CreateHash)
{
    std::string hashed;
    BOOST_CHECK(h.create_hash("test", hashed) == true);
    //BOOST_TEST_MESSAGE("Hashed: " << hashed); // ./build/tests --log_level=message
    BOOST_CHECK(hashed == "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08");
}

BOOST_AUTO_TEST_SUITE_END()