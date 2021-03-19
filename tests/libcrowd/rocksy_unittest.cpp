#include "json.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "rocksy.hpp"
using namespace Crowd;

class RocksyT : public Rocksy {};

BOOST_FIXTURE_TEST_SUITE(RocksyTests, RocksyT)

// use any protected methods inside your tests
BOOST_AUTO_TEST_CASE(PathTest)
{
    BOOST_CHECK(boost::algorithm::ends_with(usersdb_folder_path, ".config/onzehub/usersdb"));
}

BOOST_AUTO_TEST_CASE(Get)
{
    // BOOST_TEST_MESSAGE("Get: " << RocksyT::Get(1)); // ./build/tests --log_level=message
    std::string key = "1";
    BOOST_CHECK(RocksyT::Get(key) == "");
}

BOOST_AUTO_TEST_CASE(PutGetDelete)
{
    std::string key = "2";
    std::string value = "test2";
    BOOST_CHECK(RocksyT::Put(key, value) == true);
    BOOST_CHECK(RocksyT::Get(key) == "test2");
    BOOST_CHECK(RocksyT::Delete(key) == true);
    //BOOST_TEST_MESSAGE("Delete: " << RocksyT::Get("2"));
    BOOST_CHECK(RocksyT::Get(key) == "");
    RocksyT::Delete(key);
}

BOOST_AUTO_TEST_CASE(FindChosenOne)
{
    std::string key1 = "1";
    std::string key2 = "2";
    std::string key3 = "3";
    std::string value = "test2";
    BOOST_CHECK(RocksyT::Put(key2, value) == true);
    BOOST_CHECK(RocksyT::FindChosenOne(key1) == "2");
    BOOST_CHECK(RocksyT::FindChosenOne(key2) == "2");
    BOOST_CHECK(RocksyT::FindChosenOne(key3) == "2");
    RocksyT::Delete(key2);
}

BOOST_AUTO_TEST_CASE(FindNextPeer)
{
    std::string key1 = "1";
    std::string key2 = "2";
    std::string key3 = "3";
    std::string key5 = "5";
    std::string value2 = "test2";
    std::string value5 = "test5";
    BOOST_CHECK(RocksyT::Put(key2, value2) == true);
    BOOST_CHECK(RocksyT::Put(key5, value5) == true);
    BOOST_CHECK(RocksyT::FindNextPeer(key1) == "2");
    BOOST_CHECK(RocksyT::FindNextPeer(key3) == "5");
    RocksyT::Delete(key2);
    RocksyT::Delete(key5);
}

nlohmann::json j = nlohmann::json::parse("{ \"server\": true }");

BOOST_AUTO_TEST_CASE(FindServerPeer)
{
    std::string key1 = "1";
    std::string key2 = "2";
    std::string j_s = j.dump();
    BOOST_CHECK(RocksyT::Put(key2, j_s) == true);
    BOOST_CHECK(RocksyT::FindServerPeer(key1) == "2");
    BOOST_CHECK(RocksyT::FindServerPeer(key2) == "2");
    RocksyT::Delete(key2);
}

BOOST_AUTO_TEST_CASE(FindNextServerPeer)
{
    std::string key0 = "0";
    std::string key2 = "2";
    std::string key3 = "3";
    std::string j_s = j.dump();
    BOOST_CHECK(RocksyT::Put(key3, j_s) == true);
    BOOST_CHECK(RocksyT::FindNextServerPeer(key0) == "3");
    BOOST_CHECK(RocksyT::FindNextServerPeer(key2) == "3");
    RocksyT::Delete(key3);
}

BOOST_AUTO_TEST_CASE(CountPeersFromTo)
{
    std::string key0 = "0";
    std::string key1 = "1";
    std::string key2 = "2";
    std::string key3 = "3";
    std::string j_s = j.dump();
    RocksyT::Put(key0, j_s);
    RocksyT::Put(key1, j_s);
    RocksyT::Put(key2, j_s);
    RocksyT::Put(key3, j_s);
    //BOOST_TEST_MESSAGE("Count: " << RocksyT::CountPeersFromTo(key0, key0));
    BOOST_CHECK(RocksyT::CountPeersFromTo(key0, key3) == 3);
    BOOST_CHECK(RocksyT::CountPeersFromTo(key0, key0) == 4);
    RocksyT::Delete(key0);
    RocksyT::Delete(key1);
    RocksyT::Delete(key2);
    RocksyT::Delete(key3);
}

// TODO: reorder these tests --> after every test the entries should be deleted, which is not the caseder these tests --> after every test the entries should be deleted, which is not the case

BOOST_AUTO_TEST_SUITE_END()

