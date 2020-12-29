#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "p2p.hpp"

using namespace Crowd;

using boost::asio::ip::tcp;

class tcp_connection
  : public boost::enable_shared_from_this<tcp_connection>
{
public:
  typedef boost::shared_ptr<tcp_connection> pointer;

  static pointer create(boost::asio::io_context& io_context)
  {
    return pointer(new tcp_connection(io_context));
  }

  tcp::socket& socket()
  {
    return socket_;
  }

  void start()
  {
    do_read();
  }

private:
  tcp_connection(boost::asio::io_context& io_context)
    : socket_(io_context)
  {
  }

  void do_read()
  {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            do_write(length);
          }
        });
  }

  void do_write(std::size_t length)
  {
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            do_read();
          }
        });
  }

  tcp::socket socket_;
  enum { max_length = 128 };
  char data_[max_length];
};

class tcp_server
{
public:
  tcp_server(boost::asio::io_context& io_context)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), 1975))
  {
    start_accept();
  }

private:
  void start_accept()
  {
    tcp_connection::pointer new_connection =
      tcp_connection::create(io_context_);

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection,
          boost::asio::placeholders::error));
  }

  void handle_accept(tcp_connection::pointer new_connection,
      const boost::system::error_code& error)
  {
    if (!error)
    {
      new_connection->start();
    }

    start_accept();
  }

  boost::asio::io_context& io_context_;
  tcp::acceptor acceptor_;
};

int Tcp::server(std::string msg)
{
  try
  {
    std::cout << "11111" << std::endl;
    boost::asio::io_context io_context;
    tcp_server server(io_context);
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}




// // https://github.com/mwarning/UDP-hole-punching-examples/tree/master/example1
// int Udp::udp_server()
// {
//     const int port = 1975;

//     vector<Udp::client> clients;
//     int n = 0;

//     ip::address ip_address = ip::address_v4::any();
//     ip::udp::endpoint ep_me(ip_address, port), ep_other;
    
//     boost::asio::io_service io_service;
//     ip::udp::socket socket(io_service);
//     boost::system::error_code ec;
//     socket.open(ip::udp::v4(), ec);
//     socket.bind(ep_me);
//     if (ec)
//     {
//         // An error occurred.
//         Udp p;
//         p.die("");
//     }

//     while (true)
//     {
//         boost::array<char, 128> recv_buf;
//         socket.receive_from(boost::asio::buffer(recv_buf), ep_other, 0, ec);
//         if (ec && ec != boost::asio::error::message_size)
//             throw boost::system::system_error(ec);
        
//         std::cout << "Received packet from " << ep_other.address() << ":" << ep_other.port() << std::endl;

//         clients.push_back(Udp::client());
//         clients[n].host = ep_other.address();
//         clients[n].port = ep_other.port();
//         n++;

//         for (int i = 0; i < n; i++)
//         {
//             ep_other.address() = clients[i].host;
//             ep_other.port(clients[i].port);

//             nlohmann::json message = nlohmann::json::parse(recv_buf.data());

//             if (message["new_peer"] == "true") // reply with ip of next_peer(requesting_peer)
//             {
//                 std::cout << "Going for tcp now ..." << std::endl;
//                 //Udp::tcp_peer(ep_other.address().to_string(), recv_buf.data());
//             }       

//             // for (int j = 0; j < n; j++)
//             // {
//             //     std::cout << "Sending to " << ep_other.address() << ":" << ep_other.port() << std::endl;

//             //     std::string msg = "Message arrived!\n";
//             //     boost::system::error_code ignored_error;
//             //     socket.send_to(boost::asio::buffer(msg), ep_other, 0, ignored_error);
//             // }
//         }
//         std::cout << "Now we have " << n << " clients" << std::endl;
//     }
    
//     return 0;


//     // try
//     // {
//     //     const int port = 1975;

//     //     vector<Udp::client> clients;
//     //     int n = 0;

//     //     boost::asio::io_service io_service;

//     //     udp::socket socket(io_service, udp::endpoint(ip::address_v4::any(), port));

//     //     for (;;)
//     //     {
//     //         boost::array<char, 128> recv_buf;
//     //         udp::endpoint remote_endpoint;
//     //         boost::system::error_code error;
//     //         socket.receive_from(boost::asio::buffer(recv_buf),
//     //             remote_endpoint, 0, error);

//     //         if (error && error != boost::asio::error::message_size)
//     //             throw boost::system::system_error(error);

//     //         std::cout << "Received packet from " << remote_endpoint.address() << ":" << remote_endpoint.port() << std::endl;

//     //         clients.push_back(Udp::client());
//     //         clients[n].host = remote_endpoint.address();
//     //         clients[n].port = remote_endpoint.port();
//     //         n++;

//     //         for (int i = 0; i < n; i++)
//     //         {
//     //             remote_endpoint.address() = clients[i].host;
//     //             remote_endpoint.port(clients[i].port);

//     //             nlohmann::json message = nlohmann::json::parse(recv_buf.data());

//     //             if (message["new_peer"] == "true") // reply with ip of next_peer(requesting_peer)
//     //             {
//     //                 std::cout << "Going for tcp now ..." << std::endl;
//     //                 Udp::tcp_peer(remote_endpoint.address().to_string(), recv_buf.data());
//     //             }

//     //         // std::string message = make_daytime_string();

//     //         // boost::system::error_code ignored_error;
//     //         // socket.send_to(boost::asio::buffer(message),
//     //         //     remote_endpoint, 0, ignored_error);
//     //         }
//     //     }
//     // }
//     // catch (std::exception& e)
//     // {
//     //     std::cerr << e.what() << std::endl;
//     // }

//     // return 0;
// }



//         // // std::cout << "test " << recv_buf.data() << "te" << std::endl; // TODO: parse ip address here
//         // // boost::array<char, 128> msg = {"Hi"};
//         // // if (recv_buf == msg)
//         // // {
//         // //     std::cout << "Great!" << std::endl;
//         // // }

//         // // add the ip to the message
//         // // std::string message = recv_buf.data();
//         // // nlohmann::json message_j = nlohmann::json::parse(message);
//         // // message_j["ip_new_peer"] = ep_other.address().to_string();
//         // // message = message_j.dump();
//         // // std::copy(message.begin(), message.end(), recv_buf.data());
//         // // Protocol p;
//         // // p.response_hello(recv_buf);

//         // communicate latest_block to new_peer
//         // Protocol pr;
//         // boost::system::error_code ignored_error;
//         // socket.send_to(boost::asio::buffer(pr.latest_block()), ep_other, 0, ignored_error);

//         // communicate_to_all presence of new_peer
//         // - divide uint32::max into total_amount_of_peers^(1/3) parts and send the peer above those 5 partstarts a message
//         // - FindNextPeer(hash) for those resulting hashes
//         // - send_to() those peers the message
//         // std::string message = recv_buf.data();
//         // nlohmann::json message_j = nlohmann::json::parse(message);
//         // std::string hash_of_new_peer = message_j["hash_of_new_peer"];
//         // Poco po;
//         // Hash h;
//         // for (uint32_t i = static_cast<uint32_t>(std::stoul(hash_of_new_peer));
//         //      i < (static_cast<uint32_t>(std::stoul(hash_of_new_peer)) + std::numeric_limits<uint32_t>::max());
//         //      i = i + ceil(std::numeric_limits<uint32_t>::max()/pow(po.TotalAmountOfPeers(), (1.0/3.0))))
//         // {
//         //     // FindNextPeer()--> get ip address --> send_to message --> hash_of_new_peer = next_peer
//         //     std::string next_peer = po.FindNextPeer(std::to_string(i));
//         //     std::string db_value_next_peer = po.Get(next_peer);
//         //     nlohmann::json db_value_next_peer_j = nlohmann::json::parse(db_value_next_peer);
//         //     std::string ip_address = db_value_next_peer_j["ip"];
//         //     ep_other.address() = ip::address_v4::from_string(ip_address);
//         //     ep_other.port(port);
//         //     socket.send_to(boost::asio::buffer(message), ep_other, 0, ignored_error);
//         // }
