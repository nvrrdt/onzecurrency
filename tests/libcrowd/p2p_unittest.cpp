// #include <boost/test/unit_test.hpp>
// #include <boost/algorithm/string/predicate.hpp>

// #include "p2p.hpp"
// #include "p2p_client.cpp"
// using namespace Crowd;

// namespace utf = boost::unit_test;

// https://stackoverflow.com/questions/6778496/how-to-do-unit-testing-on-private-members-and-methods-of-c-classes

// namespace unit_test {
// struct FooTester
// {
//   static void somePrivateMethod(p2p_client& p2p, boost::system::error_code ec)
//   {
//     p2p.handle_read(ec);
//   }
// };
// }

// BOOST_AUTO_TEST_SUITE(FooTest);
// BOOST_AUTO_TEST_CASE(TestSomePrivateMethod)
// {
//     boost::asio::io_context io_context;
//     tcp::resolver resolver(io_context);
//     tcp::resolver::results_type endpoints;
//     endpoints = resolver.resolve("13.58.174.105", "1975");
//     p2p_client p2p(io_context, endpoints);

//     boost::system::error_code ec;
//     ec.message() = boost::system::errc::success;

//     BOOST_CHECK_EQUAL(unit_test::FooTester::somePrivateMethod(p2p, ec), true); // maybe it didn't compile because of a circular reference
                                                                                  // try to comment #include "p2p.hpp"

// }
// BOOST_AUTO_TEST_SUITE_END();

