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
    std::map<std::string, uint32_t> result1;
    result1["layers"] = 1;
    result1["remainder"] = 10;
    result1["peersinremainder"] = 1;
    result1["overshoot"] = 10;
    BOOST_CHECK(p.layer_management("10") == result1);

    std::map<std::string, uint32_t> result2;
    result2["layers"] = 2;
    result2["remainder"] = 2;
    result2["peersinremainder"] = 2;
    result2["overshoot"] = 2;
    BOOST_CHECK(p.layer_management("102") == result2);

    std::map<std::string, uint32_t> result3;
    result3["layers"] = 5;
    result3["remainder"] = 2202002;
    result3["peersinremainder"] = 13;
    result3["overshoot"] = 1101191902;
    BOOST_CHECK(p.layer_management("1202202002") == result3);
}

BOOST_AUTO_TEST_SUITE_END()
