#include "json.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "poco.hpp"
using namespace Crowd;

class PocoT : public Poco {};

BOOST_FIXTURE_TEST_SUITE(DbFolderPath, PocoT)

// use any protected methods inside your tests
BOOST_AUTO_TEST_CASE(PathTest)
{
    BOOST_CHECK(boost::algorithm::ends_with(usersdb_folder_path, ".config/onzehub/usersdb"));
}
BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(PocoTest)

Poco p;

BOOST_AUTO_TEST_CASE(Get)
{
    //BOOST_TEST_MESSAGE("Get: " << p.Get(1)); // ./build/tests --log_level=message
    BOOST_CHECK(p.Get(1) == "");
}

BOOST_AUTO_TEST_CASE(PutGetDelete)
{
    BOOST_CHECK(p.Put(2, "test2") == true);
    BOOST_CHECK(p.Get(2) == "test2");
    BOOST_CHECK(p.Delete(2) == true);
    //BOOST_TEST_MESSAGE("Delete: " << p.Get(2));
    BOOST_CHECK(p.Get(2) == "");
}

BOOST_AUTO_TEST_CASE(FindChosenOne)
{
    BOOST_CHECK(p.Put(2, "test2") == true);
    BOOST_CHECK(p.FindChosenOne(1) == 2);
    BOOST_CHECK(p.FindChosenOne(2) == 2);
    BOOST_CHECK(p.FindChosenOne(3) == 2);
}

BOOST_AUTO_TEST_CASE(FindNextPeer)
{
    BOOST_CHECK(p.Put(2, "test2") == true);
    BOOST_CHECK(p.Put(5, "test5") == true);
    BOOST_CHECK(p.FindNextPeer(1) == 2);
    BOOST_CHECK(p.FindNextPeer(3) == 5);
}

nlohmann::json j = nlohmann::json::parse("{ \"upnp\": true }");

BOOST_AUTO_TEST_CASE(FindUpnpPeer)
{
    BOOST_CHECK(p.Put(2, j.dump()) == true);
    BOOST_CHECK(p.FindUpnpPeer(1) == 2);
    BOOST_CHECK(p.FindUpnpPeer(2) == 2);
}

BOOST_AUTO_TEST_CASE(FindNextUpnpPeer)
{
    BOOST_CHECK(p.Put(3, j.dump()) == true);
    BOOST_CHECK(p.FindNextUpnpPeer(0) == 2);
    BOOST_CHECK(p.FindNextUpnpPeer(2) == 3);
}

BOOST_AUTO_TEST_SUITE_END()

// TODO: reorder these tests --> after every test the entries should be deleted, which is not the case