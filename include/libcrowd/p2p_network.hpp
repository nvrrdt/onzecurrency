#pragma once

#include <stdio.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <stack>
#include <memory>
#include <deque>

#include "p2p_message.hpp"
#include "json.hpp"
#include "crypto.hpp"
#include "prev_hash.hpp"
#include "rocksy.hpp"
#include "p2p.hpp"
#include "auth.hpp"
#include "intro_msg_mat.hpp"
#include "poco_crowd.hpp"
#include "poco_coin.hpp"
#include "merkle_tree.hpp"
#include "all_hashes_mat.hpp"
#include "block_matrix.hpp"
#include "synchronisation.hpp"
#include "print_or_log.hpp"

// This is the port for the p2p_server and p2p_client
#define PORT ("1975")

namespace Crowd
{
    class P2pNetwork
    {
    public:
        int p2p_server();
        int p2p_client(std::string ip_s, std::string message);
        std::vector<std::string> split(const std::string& str, int splitLength);

        int p2p_client_impl(std::string ip_s, std::string message);

        static void set_closed_client(std::string closed)
        {
            closed_client_ = closed;
        }
        static void set_ip_new_co(std::string ip)
        {
            ip_new_co_ = ip;
        }

        static std::string get_closed_client()
        {
            return closed_client_;
        }
        static std::string get_ip_new_co()
        {
            return ip_new_co_;
        }
        
        static void set_quit_server_req(bool quit)
        {
            quit_server_ = quit;
        }
        static bool is_connected_to_server(std::string ip_s)
        {
            for (int i = 0; i < connected_to_server_.size(); i++)
            {
Common::Print_or_log pl;
pl.handle_print_or_log({"____0001234", ip_s, " ss ", connected_to_server_.at(i)});
                if (ip_s == connected_to_server_.at(i))
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(250));

                    return true;
                }

                if (i = connected_to_server_.size() - 1) return false;
            }

            return false;
        }

        static void add_to_p2p_clients_from_other_thread(std::string ip, std::string message)
        {
            std::pair<std::string, std::string> p;
            p.first = ip;
            p.second = message;

            p2p_clients_from_other_thread_.push_back(p);
        }
    private:
        static bool get_quit_server_req()
        {
            return quit_server_;
        }
    private:
        static std::string closed_client_;
        static std::string ip_new_co_;

        static bool quit_server_;
    private:
        static void add_to_connected_to_server(std::string connected_peer)
        {
            connected_to_server_.push_back(connected_peer);
        }

        static void remove_from_connected_to_server(std::string connected_peer)
        {
            for (int i = 0; i < connected_to_server_.size(); i++)
            {
                if (connected_to_server_.at(i) == connected_peer) connected_to_server_.erase(connected_to_server_.begin() + i);
            }
        }

        static std::vector<std::string> get_connected_to_server()
        {
            return connected_to_server_;
        }

        static void add_to_client_calls(std::string ip_s)
        {
            client_calls_.push_back(ip_s);
        }

        static void remove_from_client_calls(std::string ip_s)
        {
            for (int i = 0; i < client_calls_.size(); i++)
            {
                if (client_calls_.at(i) == ip_s) client_calls_.erase(client_calls_.begin() + i);
            }
        }

        static std::vector<std::string> get_client_calls()
        {
            return client_calls_;
        }

        static void do_p2p_clients_from_other_thread()
        {
            P2pNetwork pn;

            for (auto& el: p2p_clients_from_other_thread_)
            {
                pn.p2p_client(el.first, el.second);
            }

            p2p_clients_from_other_thread_.clear();
        }

        static std::vector<std::string> connected_to_server_;
        static std::vector<std::string> client_calls_;
        static std::vector<std::pair<std::string, std::string>> p2p_clients_from_other_thread_;
    };

    typedef std::deque<p2p_message> p2p_message_queue;
    class P2pClient
    {
    public:
        P2pClient(boost::asio::io_context &io_context, const tcp::resolver::results_type &endpoints);
        void write(const p2p_message &msg);
        void close();
    private:
        void do_connect(const tcp::resolver::results_type &endpoints);
        void do_read_header();
        void do_read_body();
        void do_write();

        void handle_read_client(p2p_message read_msg_client);

        void register_for_nat_traversal_client(nlohmann::json buf_j);
        void connect_to_nat_client(nlohmann::json buf_j);
        void connect_true_client(nlohmann::json buf_j);
        void new_peer_client(nlohmann::json buf_j);
        void new_co_client(nlohmann::json buf_j);
        void your_full_hash_client(nlohmann::json buf_j);
        void hash_comparison_client(nlohmann::json buf_j);
        void close_same_conn_client(nlohmann::json buf_j);
        void close_this_conn_client(nlohmann::json buf_j);
        void close_this_conn_and_create_client(nlohmann::json buf_j);
        void send_first_block_received_client(nlohmann::json buf_j);
        void update_me_client(nlohmann::json buf_j);

        void set_resp_msg_client(std::string msg);
    private:
        boost::asio::io_context &io_context_;
        tcp::socket socket_;
        p2p_message read_msg_;
        p2p_message_queue write_msgs_;

        std::string buf_client_;
        Poco::IntroMsgVec intro_msg_vec_;
        Poco::IpHEmail ip_hemail_vec_;
    };

    class P2pServer
    {
    public:
        P2pServer(boost::asio::io_context &io_context, const tcp::endpoint &endpoint);
    private:
        void do_accept();
    private:
        tcp::acceptor acceptor_;
    };

    class P2pSession : public std::enable_shared_from_this<P2pSession>
    {
    public:
        P2pSession(tcp::socket socket);
        void start();
        void deliver(const p2p_message &msg);
    private:
        void do_read_header();
        void do_read_body();
        void do_write();

        void handle_read_server(p2p_message read_msg_server);

        void register_for_nat_traversal(nlohmann::json buf_j);
        void connect_to_nat(nlohmann::json buf_j);
        void intro_peer(nlohmann::json buf_j);
        void new_peer(nlohmann::json buf_j);
        void intro_prel_block(nlohmann::json buf_j);
        void new_prel_block(nlohmann::json buf_j);
        void intro_final_block(nlohmann::json buf_j);
        void new_final_block(nlohmann::json buf_j);
        void your_full_hash(nlohmann::json buf_j);
        void hash_comparison(nlohmann::json buf_j);
        void intro_online(nlohmann::json buf_j);
        void new_online(nlohmann::json buf_j);
        void update_you_server(nlohmann::json buf_j);

        void set_resp_msg_server(std::string msg);
    private:
        tcp::socket socket_;
        p2p_message read_msg_;
        p2p_message_queue write_msgs_;

        std::string buf_server_;
        Poco::IntroMsgVec intro_msg_vec_;
        Poco::IpHEmail ip_hemail_vec_;
    };
}