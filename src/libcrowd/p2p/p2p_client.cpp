#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "p2p_message.hpp"
#include "p2p.hpp"

using namespace Crowd;
using boost::asio::ip::tcp;

typedef std::deque<p2p_message> p2p_message_queue;

class p2p_client
{
public:
    p2p_client(boost::asio::io_context &io_context,
                const tcp::resolver::results_type &endpoints)
        : io_context_(io_context),
          socket_(io_context)
    {
        do_connect(endpoints);
    }

    void write(const p2p_message &msg)
    {
        boost::asio::post(io_context_,
                          [this, msg]() {
                              bool write_in_progress = !write_msgs_.empty();
                              write_msgs_.push_back(msg);
                              if (!write_in_progress)
                              {
                                  do_write();
                              }
                          });
    }

    void close()
    {
        boost::asio::post(io_context_, [this]() { socket_.close(); });
    }

private:
    void do_connect(const tcp::resolver::results_type &endpoints)
    {
        boost::asio::async_connect(socket_, endpoints,
                                   [this](boost::system::error_code ec, tcp::endpoint) {
                                       if (!ec)
                                       {
                                           do_read_header();
                                       }
                                   });
    }

    void do_read_header()
    {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), p2p_message::header_length),
                                [this](boost::system::error_code ec, std::size_t /*length*/) {
                                    if (!ec && read_msg_.decode_header())
                                    {
                                        do_read_body();
                                    }
                                    else
                                    {
                                        socket_.close();
                                    }
                                });
    }

    void do_read_body()
    {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                [this](boost::system::error_code ec, std::size_t /*length*/) {
                                    if (!ec)
                                    {
                                        std::cout.write(read_msg_.body(), read_msg_.body_length());
                                        std::cout << "\n";
                                        do_read_header();
                                    }
                                    else
                                    {
                                        socket_.close();
                                    }
                                });
    }

    void do_write()
    {
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(write_msgs_.front().data(),
                                                     write_msgs_.front().length()),
                                 [this](boost::system::error_code ec, std::size_t /*length*/) {
                                     if (!ec)
                                     {
                                         write_msgs_.pop_front();
                                         if (!write_msgs_.empty())
                                         {
                                             do_write();
                                         }
                                     }
                                     else
                                     {
                                         socket_.close();
                                     }
                                 });
    }

private:
    boost::asio::io_context &io_context_;
    tcp::socket socket_;
    p2p_message read_msg_;
    p2p_message_queue write_msgs_;
};

std::vector<std::string> split(const std::string& str, int splitLength)
{
   int NumSubstrings = str.length() / splitLength;
   std::vector<std::string> ret;

   for (auto i = 0; i < NumSubstrings; i++)
   {
        ret.push_back(str.substr(i * splitLength, splitLength));
   }

   // If there are leftover characters, create a shorter item at the end.
   if (str.length() % splitLength != 0)
   {
        ret.push_back(str.substr(splitLength * NumSubstrings));
   }


   return ret;
}

std::string Tcp::client(std::string srv_ip, std::string peer_ip, std::string message, std::string pub_key)
{
    try
    {
        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("13.58.174.105", "1975");
        p2p_client c(io_context, endpoints);

        std::thread t([&io_context]() { io_context.run(); });

        std::vector<std::string> splitted = split(message, p2p_message::max_body_length);

        for (int i = 0; i < splitted.size(); i++)
        {
            char s[p2p_message::max_body_length + 1];
            strncpy(s, splitted[i].c_str(), sizeof(s));
            p2p_message msg;
            msg.body_length(std::strlen(s));
            std::memcpy(msg.body(), s, msg.body_length());
            msg.encode_header();
            c.write(msg);

        }

        for (;;){}
        // char line[p2p_message::max_body_length + 1];
        // while (std::cin.getline(line, p2p_message::max_body_length + 1))
        // {
        //     p2p_message msg;
        //     msg.body_length(std::strlen(line));
        //     std::memcpy(msg.body(), line, msg.body_length());
        //     msg.encode_header();
        //     c.write(msg);
        // }

        c.close();
        t.join();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return "0";
}