#include <iostream>
#include <string.h>
#include <errno.h>
#include <json.hpp>
#include <poco.hpp>
#include "hash.hpp"
#include <limits>

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
        boost::array<char, 128> recv_buf;
        socket.receive_from(boost::asio::buffer(recv_buf), ep_other, 0, ec);
        if (ec && ec != boost::asio::error::message_size)
            throw boost::system::system_error(ec);
        
        std::cout << "Received packet from " << ep_other.address() << ":" << ep_other.port() << std::endl;

        clients.push_back(Udp::client());
        clients[n].host = ep_other.address();
        clients[n].port = ep_other.port();
        n++;

        // // std::cout << "test " << recv_buf.data() << "te" << std::endl; // TODO: parse ip address here
        // // boost::array<char, 128> msg = {"Hi"};
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

        // communicate latest_block to new_peer
        Protocol pr;
        boost::system::error_code ignored_error;
        socket.send_to(boost::asio::buffer(pr.latest_block()), ep_other, 0, ignored_error);

        // communicate_to_all presence of new_peer
        // - divide uint32::max into total_amount_of_peers^(1/3) parts and send the peer above those 5 partstarts a message
        // - FindNextPeer(hash) for those resulting hashes
        // - send_to() those peers the message
        std::string message = recv_buf.data();
        nlohmann::json message_j = nlohmann::json::parse(message);
        std::string hash_of_new_peer = message_j["hash_of_new_peer"];
        Poco po;
        Hash h;
        for (uint32_t i = static_cast<uint32_t>(std::stoul(hash_of_new_peer));
             i < (static_cast<uint32_t>(std::stoul(hash_of_new_peer)) + std::numeric_limits<uint32_t>::max());
             i = i + ceil(std::numeric_limits<uint32_t>::max()/pow(po.TotalAmountOfPeers(), (1.0/3.0))))
        {
            // FindNextPeer()--> get ip address --> send_to message --> hash_of_new_peer = next_peer
            uint32_t next_peer = po.FindNextPeer(i);
            std::string db_value_next_peer = po.Get(next_peer);
            nlohmann::json db_value_next_peer_j = nlohmann::json::parse(db_value_next_peer);
            std::string ip_address = db_value_next_peer_j["ip"];
            ep_other.address() = ip::address_v4::from_string(ip_address);
            ep_other.port(port);
            socket.send_to(boost::asio::buffer(message), ep_other, 0, ignored_error);
        }



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