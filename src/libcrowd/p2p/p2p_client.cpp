#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "p2p_message.hpp"
#include "p2p.hpp"
#include "json.hpp"
#include "crypto.h"
#include "base64.h"
#include "poco.hpp"

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

    void set_peer_hash(std::string peer_hash)
    {
        peer_hash_ = peer_hash;
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
                                        handle_read(ec);
                                        do_read_header();
                                    }
                                    else
                                    {
                                        socket_.close();
                                    }
                                });
    }

    void handle_read(boost::system::error_code ec)
    {
        if ( !read_msg_.get_eom_flag()) {
            std::string str_read_msg(read_msg_.body());
            buf_ += str_read_msg;
            handle_read(ec);
        } else {
            // process json message
            std::string str_read_msg(read_msg_.body());
            buf_ += str_read_msg;
            Tcp t;
            buf_ = t.remove_trailing_characters(buf_);

            nlohmann::json buf_j = nlohmann::json::parse(buf_);
            if (ec)
                throw boost::system::system_error(ec); // Some other error.
            else if (buf_j["register"] == "ack")
            {
                // TODO: what if there was no response from the server?

                std::cout << "Ack for registering this client to a server" << std::endl;
            } else if (buf_j["req"] == "connect")
            {
                if (buf_j["connect"] == "ok")
                {
                    std::cout << "connect = ok" << std::endl;
                    nlohmann::json message_j;
                    message_j["connect"] = "true";

                    // Establishing NAT Traversal
                    // TODO: needs to be tested that there is really a connection between the two peers
                    if (buf_j["id_from"] == "nvrrdt_from") // TODO: change nvrrdt to my_id/my_hash/my_ip
                    {
                        std::cout << "message send to id_to from id_from" << std::endl;
                        t.client("", buf_j["ip_to"], buf_j["hash_to"], message_j.dump(), "pub_key");
                        t.server("test");
                    }
                    else
                    {
                        std::cout << "message send to id_from from id_to" << std::endl;
                        t.client("", buf_j["ip_from"], buf_j["hash_from"], message_j.dump(), "pub_key");
                        t.server("test");
                    }
                }
            } else if (buf_j["connect"] == "true")
            {
                std::cout << "connection established" << std::endl;
            }

            buf_ = ""; // reset buffer, otherwise nlohmann receives an incorrect string
        }
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
    std::string buf_;
    p2p_message resp_msg_;
    std::string peer_hash_;
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

bool Tcp::set_close_client(bool close) // preparation for closing the client should still be used somewhere
{
    return close;
}

std::string Tcp::client(std::string srv_ip, std::string peer_ip, std::string peer_hash, std::string message, std::string pub_key)
{
    try
    {
        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints;
        if (peer_ip == "")
        {
            endpoints = resolver.resolve("13.58.174.105", "1975");
        }
        else
        {
            endpoints = resolver.resolve(peer_ip, "1975");
        }
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
            i == splitted.size() - 1 ? msg.encode_header(1) : msg.encode_header(0); // 1 indicates end of message eom, TODO perhaps a set_eom_flag(true) instead of an int

            if (peer_hash != "") c.set_peer_hash(peer_hash);
            c.write(msg);

        }

        for (;;)
        {
            if (close_client_ == true)
            {
                c.close();
                t.join();
                break;
            }
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return "0";
}