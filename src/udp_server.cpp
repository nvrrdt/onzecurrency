#include <iostream>
#include <string.h>
#include <errno.h>

#include "p2p.hpp"

using namespace Crowd;

struct P2p::client
{
    boost::asio::ip::address host;
    short port;
};

bool P2p::udp_client()
{
    struct P2p::client clients[512];
    int n = 0;

    unsigned short port_num = 1975;
    ip::address ip_address = ip::address_v4::any();
    ip::udp::endpoint ep_me(ip_address, port_num), ep_other;
    
    boost::asio::io_service io_service;
    ip::udp::socket socket(io_service);
    boost::system::error_code ec;
    socket.open(ip::udp::v4(), ec);
    socket.bind(ep_me);
    if (ec)
    {
        // An error occurred.
        P2p::die();
    }

    while (true)
    {
        boost::array<char, 32> recv_buf;
        socket.receive_from(boost::asio::buffer(recv_buf), ep_other, 0, ec);
        if (ec && ec != boost::asio::error::message_size)
            throw boost::system::system_error(ec);
        
        std::cout << "Received packet from " << ep_other.address() << ":" << ep_other.port() << std::endl;

        clients[n].host = ep_other.address();
        clients[n].port = ep_other.port();
        n++;

        for (int i = 0; i < n; i++)
        {
            ep_other.address() = clients[i].host;
            strncpy((char *) ep_other.port(), (char *) clients[i].port, sizeof(short int));
            
            for (int j = 0; j < n; j++)
            {
                std::cout << "Sending to " << ep_other.address() << ":" << ep_other.port() << std::endl;

                std::string msg = "Message arrived!";
                boost::system::error_code ignored_error;
                socket.send_to(boost::asio::buffer(msg),
                    ep_other, 0, ignored_error);
            }
        }
        std::cout << "Now we have " << n << " clients" << std::endl;
    }
    
    return true;
}