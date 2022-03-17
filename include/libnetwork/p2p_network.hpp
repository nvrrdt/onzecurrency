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

namespace Network
{
    class P2pNetwork
    {
    public:
        int p2p_server();
        int p2p_client(std::string ip_s, std::string message);
        std::vector<std::string> split(const std::string& str, int splitLength);

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
    private:
        static std::string closed_client_;
        static std::string ip_new_co_;
    };

    typedef std::deque<p2p_message> p2p_message_queue;
    class P2pClient
    {
    public:
        P2pClient(boost::asio::io_context &io_context, const tcp::resolver::results_type &endpoints);
        void write(const p2p_message &msg);
        void close();

        void set_resp_msg_client(std::string msg);
    private:
        void do_connect(const tcp::resolver::results_type &endpoints);
        void do_read_header();
        void do_read_body();
        void do_write();

        void handle_read_client(p2p_message read_msg_client);

        // Common
        void update_me_client(nlohmann::json buf_j);
        void update_you_client(nlohmann::json buf_j);

        // Crowd client
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
        void new_co_offline_client(nlohmann::json buf_j);
        void new_co_online_client(nlohmann::json buf_j);

    private:
        boost::asio::io_context &io_context_;
        const tcp::resolver::results_type &endpoints_;
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

        void set_resp_msg_server(std::string msg);
    private:
        void do_read_header();
        void do_read_body();
        void do_write();

        void handle_read_server(p2p_message read_msg_server);

        // Crowd server
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
        void intro_offline(nlohmann::json buf_j);
        void new_offline(nlohmann::json buf_j);
        void update_you_server(nlohmann::json buf_j);

        // Coin server
        void hello_tx(nlohmann::json buf_j);
        void intro_tx(nlohmann::json buf_j);
        void new_tx(nlohmann::json buf_j);
        void hello_reward(nlohmann::json buf_j);
        void intro_reward(nlohmann::json buf_j);
        void new_reward(nlohmann::json buf_j);
        void start_block_creation_thread();
        void get_sleep_and_create_block_server_c();
        void intro_block_c(nlohmann::json buf_j);
        void hash_comparison_c(nlohmann::json buf_j);
        void new_block_c(nlohmann::json buf_j);
        void intro_online_c(nlohmann::json buf_j);
        void new_online_c(nlohmann::json buf_j);
    protected:
        tcp::socket socket_;
    private:
        p2p_message read_msg_;
        p2p_message_queue write_msgs_;

        std::string buf_server_;
        Poco::IntroMsgVec intro_msg_vec_;
        Poco::IpHEmail ip_hemail_vec_;
    };
}