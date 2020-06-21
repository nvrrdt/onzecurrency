#include <iostream>
#include <string.h>
#include <errno.h>

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

        // std::cout << "test " << recv_buf.data() << "te" << std::endl; // TODO: parse ip address here
        // boost::array<char, 32> msg = {"Hi"};
        // if (recv_buf == msg)
        // {
        //     std::cout << "Great!" << std::endl;
        // }

        Protocol p;
        p.response_hello(recv_buf);

        for (int i = 0; i < n; i++)
        {
            ep_other.address() = clients[i].host;
            ep_other.port(clients[i].port);
                        
            for (int j = 0; j < n; j++)
            {
                std::cout << "Sending to " << ep_other.address() << ":" << ep_other.port() << std::endl;

                std::string msg = "Message arrived!\n";
                boost::system::error_code ignored_error;
                socket.send_to(boost::asio::buffer(msg), ep_other, 0, ignored_error);
            }
        }
        std::cout << "Now we have " << n << " clients" << std::endl;
    }
    
    return 0;
}