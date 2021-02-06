#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "p2p.hpp"
using namespace Crowd;

// class PocoT : public Poco {};

// BOOST_FIXTURE_TEST_SUITE(DbFolderPath, PocoT)

// // use any protected methods inside your tests
// BOOST_AUTO_TEST_CASE(PathTest)
// {
//     BOOST_CHECK(boost::algorithm::ends_with(usersdb_folder_path, ".config/onzehub/usersdb"));
// }
// BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(ProtocolTest)

Protocol p;

BOOST_AUTO_TEST_CASE(LayerManagement)
{
    std::map<uint32_t, uint256_t>result1;
    for (int i = 1; i <= 100; i++)
    {
        if (i <= 10)
        {
            result1[i] = 1;
        }
        else
        {
            result1[i] = 0;
        }
    }
    uint256_t s = 10;
    BOOST_CHECK(p.layers_management(s) == result1);

    std::map<uint32_t, uint256_t> result2;
    for (int i = 1; i <= 100; i++)
    {
        if (i <= 1)
        {
            result2[i] = 3;
        }
        else
        {
            result2[i] = 1;
        }
    }
    s = 102;
    BOOST_CHECK(p.layers_management(s) == result2);

    std::map<uint32_t, uint256_t> result3;
    for (int i = 1; i <= 100; i++)
    {
        if (i <= 11)
        {
            result3[i] = 101010101;
        }
        else if (i == 12)
        {
            result3[i] = 2202003;
        }
        else
        {
            result3[i] = 1010101;
        }
    }
    s = 1202202002;
    BOOST_CHECK(p.layers_management(s) == result3);
}

BOOST_AUTO_TEST_SUITE_END()
