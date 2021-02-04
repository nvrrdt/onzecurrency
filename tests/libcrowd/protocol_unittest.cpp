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
    std::map<std::string, uint64_t> result1;
    result1["numberlayers"] = 1;
    result1["numbermaxbucketslastlayer"] = 10;
    result1["contentsmaxbucketslastlayer"] = 1;
    result1["numberminbucketslastlayer"] = 90;
    result1["contentsminbucketslastlayer"] = 0;
    result1["overshootlastlayer"] = 10;
    std::string s = "10";
    BOOST_CHECK(p.layers_management(s) == result1);

    std::map<std::string, uint64_t> result2;
    result2["numberlayers"] = 2;
    result2["numbermaxbucketslastlayer"] = 2;
    result2["contentsmaxbucketslastlayer"] = 1;
    result2["numberminbucketslastlayer"] = 9998;
    result2["contentsminbucketslastlayer"] = 0;
    result2["overshootlastlayer"] = 2;
    s = "102";
    BOOST_CHECK(p.layers_management(s) == result2);

    std::map<std::string, uint64_t> result3;
    result3["numberlayers"] = 5;
    result3["numbermaxbucketslastlayer"] = 1101191902;
    result3["contentsmaxbucketslastlayer"] = 1;
    result3["numberminbucketslastlayer"] = 8898808098;
    result3["contentsminbucketslastlayer"] = 0;
    result3["overshootlastlayer"] = 1101191902;
    s = "1202202002";
    BOOST_CHECK(p.layers_management(s) == result3);
}

BOOST_AUTO_TEST_SUITE_END()
