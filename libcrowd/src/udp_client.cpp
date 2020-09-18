#include "json.hpp"
#include "poco.hpp"

#include "p2p.hpp"

using namespace Crowd;

//#define PORT "1975"
//#define SRV_IP "127.0.0.1" //"141.135.77.90"

int Udp::udp_client(std::string srv_ip, std::string message)
{
    short port = 1975;

    vector<Udp::client> buf;
    vector<Udp::client> server;
    vector<Udp::client> peers;
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

    server.push_back(Udp::client());
    server[0].host = ep_other.address();
    server[0].port = ep_other.port();

    boost::array<char, 128> send_buf;
    std::copy(message.begin(), message.end(), send_buf.data());
    socket.send_to(boost::asio::buffer(send_buf), ep_other); // send message to upnp_peer

    nlohmann::json message_json = nlohmann::json::parse(message);
    if (message_json["upnp"] == true)
    {
        return 0;
    }

    while (true)
    {
        boost::array<char, 128> recv_buf;
        boost::system::error_code ec;
        size_t len = socket.receive_from(boost::asio::buffer(recv_buf), ep_me, 0, ec);
        if (ec && ec != boost::asio::error::message_size)
            throw boost::system::system_error(ec);

        std::cout << "Received packet from " << ep_me.address() << ":" << ep_me.port() << std::endl;

        buf[0].host = ep_me.address();
        buf[0].port = ep_me.port();

        if (server[0].host == ep_other.address() && server[0].port == ep_other.port())
        {
            int f = 0;
            for (int i = 0; i < n && f == 0; i++)
            {
                if (peers[i].host == buf[0].host && peers[i].port == buf[0].port)
                {
                    f = 1;
                }
            }
            if (f == 0)
            {
                peers.push_back(Udp::client());
                peers[n].host = buf[0].host;
                peers[n].port = buf[0].port;
                n++;
            }
            ep_other.address() = buf[0].host;
            ep_other.port(buf[0].port);
            std::cout << "Added peer " << ep_other.address() << ":" << ep_other.port() << std::endl;
            std::cout << "Now we have " << n << " peers" << std::endl;
            for (int k = 0; k < 10; k++)
            {
                for (int i = 0; i < n; i++)
                {
                    ep_other.address() = peers[i].host;
                    ep_other.port(peers[i].port);
                    // Payload irrelevant
                    boost::array<char, 128> send_buf  = { "Hi" };
                    boost::system::error_code ignored_error;
                    socket.send_to(boost::asio::buffer(send_buf), ep_other, 0, ignored_error);
                }
            }
        }
        else
        {
            /**
             * Read the message from the peer
             * If hello: hash, upnp, ip and update rocksdb
             */

            std::string message = recv_buf.data();
            nlohmann::json message_j = nlohmann::json::parse(message);
            std::string hash_of_new_peer = message_j["hash_of_new_peer"];
            //std::string ip_of_new_peer = message_j["ip"];
            //std::string upnp_of_new_peer = message_j["upnp"];
            message_j.erase("hash_of_new_peer");

            Poco p;
            p.Put(static_cast<uint32_t>(std::stoul(hash_of_new_peer)), message_j.dump());



            // The datagram came from a peer
            for (int i = 0; i < n; i++)
            {
                // Identify which peer it came from
                if (peers[i].host == buf[0].host && peers[i].port == (short)(buf[0].port))                {
                    // And do something useful with the received payload
                    std::cout << "Received from peer " << i << std::endl;
                    break;
                }
            }
        }
        
    }

    return 0;
}