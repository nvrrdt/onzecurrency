// #include <iostream>
// #include <boost/array.hpp>
// #include <boost/asio.hpp>
// #include <boost/algorithm/string.hpp>

// #include "json.hpp"
// #include "poco.hpp"
// #include "p2p.hpp"

// using namespace Crowd;

// using boost::asio::ip::tcp;

// std::string Tcp::client(std::string srv_ip, std::string peer_ip, std::string message, std::string pub_key) {
//     std::string msg = "";

//     try
//     {
//         boost::asio::io_context io_context;

//         tcp::resolver resolver(io_context);
//         tcp::resolver::results_type endpoints =
//           resolver.resolve("13.58.174.105", "1975");

//         tcp::socket socket(io_context);
//         boost::asio::connect(socket, endpoints);

//         boost::system::error_code error;
//         socket.write_some(boost::asio::buffer(message), error); // TODO: send header with length to send variable length messages

//         enum { max_length = 1024 };
//         char buf[max_length];
//         size_t len = socket.read_some(boost::asio::buffer(buf), error);

//         std::string str_buf = Tcp::remove_trailing_characters(buf);

//         nlohmann::json buf_j = nlohmann::json::parse(str_buf);
//         if (error)
//             throw boost::system::system_error(error); // Some other error.
//         else if (buf_j["register"] == "ack")
//         {
//             for (;;)
//             {
//                 std::cout << "Reading after ack" << std::endl;
//                 size_t len = socket.read_some(boost::asio::buffer(buf), error);
//                 // next step is to read a request for accepting a peer's request if you're not a server
//             }
//         }

//         std::cout << buf << std::endl;
//     }
//     catch (std::exception& e)
//     {
//         std::cerr << e.what() << std::endl;
//     }

//     return msg;
// }

// std::string Tcp::remove_trailing_characters(std::string buf)
// {
//     // there are trailing chartacters that need to be trimmed,
//     // working with an header for comunicating the length of the body is probably a solution
//     std::string str_buf(buf);
//     std::size_t found = str_buf.find_last_of("}");
    
//     return str_buf.erase(found+1, sizeof(str_buf));
// }

// // Example for udp hole punching: https://github.com/mwarning/UDP-hole-punching-examples/tree/master/example1


// // for (;;)
// // {
// //     enum { max_length = 1024 };
// //     char buf[max_length];

// //     size_t len = socket.read_some(boost::asio::buffer(buf), error);

// //     if (error == boost::asio::error::eof) {
// //         break; // Connection closed cleanly by peer.
// //     }
// //     else if (error)
// //         throw boost::system::system_error(error); // Some other error.
// // }