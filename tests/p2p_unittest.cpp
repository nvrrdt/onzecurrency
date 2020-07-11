#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "p2p.hpp"
using namespace Crowd;

namespace utf = boost::unit_test;


class UdpT : public Udp {};

BOOST_FIXTURE_TEST_SUITE(UdpSuite, UdpT)

// use any protected methods inside your tests
BOOST_AUTO_TEST_CASE(TimeoutUdpClientTrue, * utf::timeout(5))
{
    udp_client("127.0.0.1", "{ \"upnp\": true }"); // Returns!
}
BOOST_AUTO_TEST_CASE(TimeoutUdpClientFalse, * utf::timeout(5))
{
    udp_client("127.0.0.1", "{ \"upnp\": false }"); // Must fail!
}
BOOST_AUTO_TEST_CASE(TimeoutUdpServer, * utf::timeout(5))
{
    udp_server(); // Must fail!
}

BOOST_AUTO_TEST_SUITE_END()
