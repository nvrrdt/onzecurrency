#include <iostream>
#include <string.h>
#include <errno.h>
#include <json.hpp>

#include "p2p.hpp"

using namespace Crowd;

int Udp::udp_server()
{
    short port = 1975;

    vector<Udp::client> clients;
    int n = 0;

    ip::address ip_address = ip::address_v4::any();
    ip::udp::endpoint ep_me(ip_address, port), ep_other;
    
    boost::asio::io_service io_service;
    ip::udp::socket socket(io_service);
    boost::system::error_code ec;
    socket.open(ip::udp::v4(), ec);
    socket.bind(ep_me);
    if (ec)
    {
        // An error occurred.
        Udp p;
        p.die();
    }

    while (true)
    {
        boost::array<char, 32> recv_buf;
        socket.receive_from(boost::asio::buffer(recv_buf), ep_other, 0, ec);
        if (ec && ec != boost::asio::error::message_size)
            throw boost::system::system_error(ec);
        
        std::cout << "Received packet from " << ep_other.address() << ":" << ep_other.port() << std::endl;

        clients.push_back(Udp::client());
        clients[n].host = ep_other.address();
        clients[n].port = ep_other.port();
        n++;

        // // std::cout << "test " << recv_buf.data() << "te" << std::endl; // TODO: parse ip address here
        // // boost::array<char, 32> msg = {"Hi"};
        // // if (recv_buf == msg)
        // // {
        // //     std::cout << "Great!" << std::endl;
        // // }

        // // add the ip to the message
        // // std::string message = recv_buf.data();
        // // nlohmann::json message_j = nlohmann::json::parse(message);
        // // message_j["ip_new_peer"] = ep_other.address().to_string();
        // // message = message_j.dump();
        // // std::copy(message.begin(), message.end(), recv_buf.data());
        // // Protocol p;
        // // p.response_hello(recv_buf);

        /**
         * - make a list of upnp_peer's clients, that might come in handy, but maybe not necessary -- DONE
         * - response_hello: communicate to new_peer the latest block && communicate_to_all new_peer
         *   --> (create new hash from peer and repeat rehashing until Roundup(amount_of_online_peers^(1/3)))
         * 
         * #include <limits>
         * std::numeric_limits<uint32_t>::max(); // should be 2^32!
         * 
         * #include <math.h>
         * ceil(2.3)
         * pow (7.0, 3.0)
         */

        // communicate latest_block to new_peer
        Protocol p;
        std::string latest_block = p.latest_block();
        boost::system::error_code ignored_error;
        socket.send_to(boost::asio::buffer(latest_block), ep_other, 0, ignored_error);

        // communicate_to_all presence of new_peer
        // - hash Roundup(amount_of_online_peers^(1/3)) times the hash_of_new_peer
        // - FindNextPeer(hash) for those resulting hashes
        // - send_to() those peers the message



        // // for (int i = 0; i < n; i++)
        // // {
        // //     ep_other.address() = clients[i].host;
        // //     ep_other.port(clients[i].port);
                        
        // //     for (int j = 0; j < n; j++)
        // //     {
        // //         std::cout << "Sending to " << ep_other.address() << ":" << ep_other.port() << std::endl;

        // //         std::string msg = "Message arrived!\n";
        // //         boost::system::error_code ignored_error;
        // //         socket.send_to(boost::asio::buffer(msg), ep_other, 0, ignored_error);
        // //     }
        // // }
        // // std::cout << "Now we have " << n << " clients" << std::endl;
    }
    
    return 0;
}