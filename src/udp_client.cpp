#include "p2p.hpp"

using namespace Crowd;

//#define PORT "1975"
//#define SRV_IP "127.0.0.1" //"141.135.77.90"

bool Udp::udp_client()
{
    short port = 1975;
    std::string srv_ip = "127.0.0.1";

    struct Udp::client buf;
    struct Udp::client server;
    struct Udp::client peers[512];
    int n = 0;

    ip::address ip_address_me = ip::address_v4::any();
    //ip::address ip_address_other = ip::address_v4::from_string(SRV_IP);
    ip::udp::endpoint ep_me(ip_address_me, port/*, ep_other(ip_address_other, PORT*/);
    
    /*boost::asio::io_service io_service;
    ip::udp::socket socket(io_service);
    boost::system::error_code ec;
    socket.open(ip::udp::v4(), ec);*/

    boost::asio::io_service io_service;
    udp::resolver resolver(io_service);
    udp::resolver::query query(udp::v4(), srv_ip, to_string(port));
    udp::endpoint ep_other = *resolver.resolve(query);

    udp::socket socket(io_service);
    socket.open(udp::v4());

    server.host = ep_other.address();
    server.port = ep_other.port();

    boost::array<char, 32> send_buf  = {"Hi"};
    socket.send_to(boost::asio::buffer(send_buf), ep_other);

    while (true)
    {
        boost::array<char, 32> recv_buf;
        size_t len = socket.receive_from(boost::asio::buffer(recv_buf), ep_me);

        std::cout.write(recv_buf.data(), len);

        if (server.host == ep_other.address() && server.port == ep_other.port())
        {
            int f = 0;
            for (int i = 0; i < n && f == 0; i++)
            {
                if (peers[i].host == buf.host && peers[i].port == buf.port)
                {
                    f = 1;
                }
            }
            if (f == 0)
            {
                peers[n].host = buf.host;
                peers[n].port = buf.port;
                n++;
            }
            ep_other.address() = buf.host;
            ep_other.port(buf.port);
            std::cout << "Added peer " << ep_other.address() << ":" << ep_other.port() << std::endl;
            std::cout << "Now we have " << n << " peers" << std::endl;
            for (int k = 0; k < 10; k++)
            {
                for (int i = 0; i < n; i++)
                {
                    ep_other.address() = peers[i].host;
                    ep_other.port(peers[i].port);
                    // Payload irrelevant
                    boost::array<char, 32> send_buf  = { "Hi" };
                    boost::system::error_code ignored_error;
                    socket.send_to(boost::asio::buffer(send_buf), ep_other, 0, ignored_error);
                }
            }
        }
        else
        {
            // The datagram came from a peer
            for (int i = 0; i < n; i++)
            {
                // Identify which peer it came from
                if (peers[i].host == buf.host && peers[i].port == (short)(buf.port))
                {
                    // And do something useful with the received payload
                    std::cout << "Received from peer " << i << std::endl;
                    break;
                }
            }
        }
        
    }

    return true;
}