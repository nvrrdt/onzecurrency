#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "p2p_message.hpp"
#include "p2p.hpp"
#include "json.hpp"
#include "poco.hpp"
#include "merkle_tree.hpp"

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

    void set_close_client(bool close) // preparation for closing the client should still be used somewhere
    {
        close_client_ = close;
    }

    bool get_close_client()
    {
        return close_client_;
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
                                        // std::cout << "do_read_body: ";
                                        // std::cout.write(read_msg_.body(), read_msg_.body_length());
                                        // std::cout << "\n";
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
        } else {
            // process json message
            std::string str_read_msg(read_msg_.body());
            buf_ += str_read_msg.substr(0, read_msg_.get_body_length());

            Tcp t;

            nlohmann::json buf_j = nlohmann::json::parse(buf_);
            if (ec)
            {
                throw boost::system::system_error(ec); // Some other error.
            }
            else if (buf_j["register"] == "ack")
            {
                // TODO: what if there was no response from the server?

                std::cout << "Ack for registering this client to a server" << std::endl;
            }
            else if (buf_j["req"] == "connect")
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
                        std::string srv_ip = "";
                        std::string peer_ip = buf_j["ip_to"];
                        std::string peer_hash = buf_j["hash_to"];
                        std::string message = message_j.dump();
                        std::string pub_key = "pub_key";
                        t.client(srv_ip, peer_ip, peer_hash, message);
                        t.server();
                    }
                    else
                    {
                        std::cout << "message send to id_from from id_to" << std::endl;
                        std::string srv_ip = "";
                        std::string peer_ip = buf_j["ip_from"];
                        std::string peer_hash = buf_j["hash_from"];
                        std::string message = message_j.dump();
                        std::string pub_key = "pub_key";
                        t.client(srv_ip, peer_ip, peer_hash, message);
                        t.server();
                    }
                }
            }
            else if (buf_j["connect"] == "true")
            {
                std::cout << "connection established" << std::endl;
            }
            else if (buf_j["req"] == "new_block")
            {
                std::cout << "new_block" << std::endl;
                // save blocks to blockchain folder

                nlohmann::json block_j = buf_j["block"].get<nlohmann::json>();
                std::string block_nr = buf_j["block_nr"];
                if (block_nr == "no blockchain present in folder") block_nr = "0";
                // std::cout << "block_s: " << buf_j["block"] << std::endl;
                // std::cout << "block_nr: " << block_nr << std::endl;

                merkle_tree mt;
                mt.save_block_to_file(block_j, block_nr);
            }
            else if (buf_j["req"] == "update_my_blocks")
            {
                std::cout << "update_my_blocks" << std::endl;
                // send blocks to peer

                Protocol proto;
                std::string my_latest_block = proto.get_last_block_nr();
                std::string req_latest_block = buf_j["block_nr"];

                nlohmann::json list_of_blocks_j = nlohmann::json::parse(proto.get_blocks_from(req_latest_block));

                uint64_t value;
                std::istringstream iss(my_latest_block);
                iss >> value;

                for (uint64_t i = 0; i <= value; i++)
                {
                    nlohmann::json block_j = list_of_blocks_j[i]["block"];
                    // std::cout << "block_j: " << block_j << std::endl;
                    nlohmann::json msg;
                    msg["req"] = "update_your_blocks";
                    std::ostringstream o;
                    o << i;
                    msg["block_nr"] = o.str();
                    msg["block"] = block_j;
                    set_resp_msg(msg.dump());
                }
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
                                         std::cout << "ec1: " << ec << std::endl;
                                         write_msgs_.pop_front();
                                         if (!write_msgs_.empty())
                                         {
                                             do_write();
                                         }
                                     }
                                     else
                                     {
                                         socket_.close();
                                         set_close_client(true);
                                         std::cout << "Connection closed!" << std::endl;
                                     }
                                 });
    }

    void set_resp_msg(std::string msg)
    {
        std::vector<std::string> splitted = split(msg, p2p_message::max_body_length);
        for (int i = 0; i < splitted.size(); i++)
        {
            char s[p2p_message::max_body_length + 1];
            strncpy(s, splitted[i].c_str(), sizeof(s));

            resp_msg_.body_length(std::strlen(s));
            std::memcpy(resp_msg_.body(), s, resp_msg_.body_length());
            i == splitted.size() - 1 ? resp_msg_.encode_header(1) : resp_msg_.encode_header(0); // 1 indicates end of message eom, TODO perhaps a set_eom_flag(true) instead of an int

            write(resp_msg_);
        }
    }

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
    
private:
    boost::asio::io_context &io_context_;
    tcp::socket socket_;
    p2p_message read_msg_;
    p2p_message_queue write_msgs_;
    std::string buf_;
    p2p_message resp_msg_;
    std::string peer_hash_;

    bool close_client_;

    friend struct ::unit_test::FooTester;
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

std::string Tcp::client(std::string &srv_ip, std::string &peer_ip, std::string &peer_hash, std::string &message)
{
    try
    {
        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints;
        endpoints = resolver.resolve(peer_ip, "1975");
        
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

        while (true) // ugly, but this client should be able to receive and send messages, it doesn't work without this while
        {
            if (c.get_close_client())
            {
                bool t = true;
                set_tcp_closed_client(t);
                break;
            }
        }
        c.close();
        t.join();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return "0";
}