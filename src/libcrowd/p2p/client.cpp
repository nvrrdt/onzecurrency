#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "json.hpp"
#include "poco.hpp"
#include "p2p.hpp"

using namespace Crowd;

using boost::asio::ip::tcp;

std::string Tcp::client(std::string srv_ip, std::string peer_ip, std::string message, std::string pub_key)
{
  std::string msg = "";

  try
  {
    boost::asio::io_context io_context;

    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints =
      resolver.resolve("127.0.0.1", "daytime");

    tcp::socket socket(io_context);
    boost::asio::connect(socket, endpoints);

    for (;;)
    {
      boost::array<char, 128> buf;
      boost::system::error_code error;

      size_t len = socket.read_some(boost::asio::buffer(buf), error);

      if (error == boost::asio::error::eof)
        break; // Connection closed cleanly by peer.
      else if (error)
        throw boost::system::system_error(error); // Some other error.

      std::cout.write(buf.data(), len);

      msg += buf.data();
    }
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return msg;
}


// // https://github.com/mwarning/UDP-hole-punching-examples/tree/master/example1
// int Udp::udp_client(std::string srv_ip, std::string peer_ip, std::string message)
// {
//     const int port = 1975;

//     vector<Udp::client> buf;
//     vector<Udp::client> server;
//     vector<Udp::client> peers;
//     int n = 0;

//     boost::asio::io_service io_service;
//     udp::resolver resolver(io_service);
//     udp::resolver::query query(srv_ip, to_string(port), boost::asio::ip::resolver_query_base::numeric_service);
//     udp::endpoint ep_other = *resolver.resolve(query);

//     udp::socket socket(io_service);
//     socket.open(udp::v6());

//     server.push_back(Udp::client());
//     server[0].host = ep_other.address();
//     server[0].port = ep_other.port();

//     boost::array<char, 128> send_buf;
//     std::copy(message.begin(), message.end(), send_buf.data());
//     std::cout << "ip: " << srv_ip << ", msg: " << send_buf.data() << std::endl;
//     socket.send_to(boost::asio::buffer(send_buf), ep_other); // send message to upnp_peer

//     // nlohmann::json message_json = nlohmann::json::parse(message);
//     // if (!message_json.is_null() && message_json["upnp"] == true)
//     // {
//     //     return 0;
//     // }

//     while (true)
//     {
//         boost::array<char, 128> recv_buf;
//         boost::system::error_code ec;
//         size_t len = socket.receive_from(boost::asio::buffer(recv_buf), ep_other, 0, ec);
//         if (ec && ec != boost::asio::error::message_size)
//             throw boost::system::system_error(ec);

//         std::cout << "Received packet from " << ep_other.address() << ":" << ep_other.port() << std::endl;

//         buf[0].host = ep_other.address();
//         buf[0].port = ep_other.port();

//         if (server[0].host == ep_other.address() && server[0].port == ep_other.port())
//         {
//             int f = 0;
//             for (int i = 0; i < n && f == 0; i++)
//             {
//                 if (peers[i].host == buf[0].host && peers[i].port == buf[0].port)
//                 {
//                     f = 1;
//                 }
//             }
//             if (f == 0)
//             {
//                 peers.push_back(Udp::client());
//                 peers[n].host = buf[0].host;
//                 peers[n].port = buf[0].port;
//                 n++;
//             }
//             ep_other.address() = buf[0].host;
//             ep_other.port(buf[0].port);
//             std::cout << "Added peer " << ep_other.address() << ":" << ep_other.port() << std::endl;
//             std::cout << "Now we have " << n << " peers" << std::endl;

//             if (recv_buf.data() == "new_peer")
//             {
//                 //Udp::tcp_peer(ep_other.address().to_string(), recv_buf.data());
//             }

//             // for (int k = 0; k < 10; k++)
//             // {
//                 // for (int i = 0; i < n; i++)
//                 // {
//                 //     ep_other.address() = peers[i].host;
//                 //     ep_other.port(peers[i].port);
//                 //     // Payload irrelevant

//                 //     Udp::tcp_peer(ep_other.address().to_string());

//                 //     // boost::array<char, 128> send_buf  = { "Hi" };
//                 //     // boost::system::error_code ignored_error;
//                 //     // socket.send_to(boost::asio::buffer(send_buf), ep_other, 0, ignored_error);
//                 // }
//             // }
//         }
//         else
//         {
//             /**
//              * Read the message from the peer
//              * If hello: hash, upnp, ip and update rocksdb
//              */

//             std::string message = recv_buf.data();
//             nlohmann::json message_j = nlohmann::json::parse(message);
//             std::string hash_of_new_peer = message_j["hash_of_new_peer"];
//             //std::string ip_of_new_peer = message_j["ip"];
//             //std::string upnp_of_new_peer = message_j["upnp"];
//             message_j.erase("hash_of_new_peer");

//             Poco p;
//             p.Put(hash_of_new_peer, message_j.dump());



//             // The datagram came from a peer
//             for (int i = 0; i < n; i++)
//             {
//                 // Identify which peer it came from
//                 if (peers[i].host == buf[0].host && peers[i].port == (short)(buf[0].port))                {
//                     // And do something useful with the received payload
//                     std::cout << "Received from peer " << i << std::endl;
//                     break;
//                 }
//             }
//         }
        
//     }

//     return 0;
// }