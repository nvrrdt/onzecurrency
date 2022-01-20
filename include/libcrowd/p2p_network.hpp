#pragma once

#include <stdio.h>
#include <string.h>
#include <enet/enet.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <stack>
#include <memory>

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

// This is the port for the p2p_server and p2p_client
#define PORT (1975)

namespace Crowd
{
    class P2pNetwork
    {
    public:
        int p2p_server();
        int p2p_client(std::string ip_s, std::string message);
        int p2p_client_impl(std::string ip_s, std::string message);
        static std::string get_closed_client()
        {
            return closed_client_;
        }
        static uint32_t get_ip_new_co()
        {
            return ip_new_co_;
        }
        static void set_quit_server_req(bool quit)
        {
            quit_server_ = quit;
        }
    protected:
        void set_resp_msg_server(std::string msg);
        void set_resp_msg_client(std::string msg);
        void set_resp_your_hash_server(enet_uint32 participant, std::string msg);
    private:
        std::vector<std::string> split(const std::string& str, int splitLength);
        void do_read_header_server(p2p_message read_msg_server);
        void do_read_body_server(p2p_message read_msg_server);
        void do_read_header_client(p2p_message read_msg_client);
        void do_read_body_client(p2p_message read_msg_client);
        void handle_read_server(p2p_message read_msg_server);
        void handle_read_client(p2p_message read_msg_client);
        static void set_closed_client(std::string closed)
        {
            closed_client_ = closed;
        }
        static void set_ip_new_co(uint32_t ip)
        {
            ip_new_co_ = ip;
        }

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

        static bool get_quit_server_req()
        {
            return quit_server_;
        }
    protected:
        ENetHost   *server_;
    private:
        char buffer_[p2p_message::max_body_length];

        ENetHost  *client_;
        ENetAddress  address_client_;
        ENetAddress  address_server_;
        ENetEvent  event_client_;
        ENetEvent  event_server_;
        ENetPeer  *peer_client_;
        ENetPacket  *packet_client_;
        ENetPacket  *packet_server_;

        std::string buf_client_;
        std::string buf_server_;
        Poco::IntroMsgVec intro_msg_vec_;
        Poco::IpHEmail ip_hemail_vec_;

        static std::string closed_client_;
        static uint32_t ip_new_co_;

        static bool quit_server_;
    private:
        static void add_to_connected_to_server(std::string connected_peer)
        {
            bool br = false;

            for (;;)
            {
                for (int i = 0; i < connected_to_server_.size(); i++)
                {
                    if (connected_peer == connected_to_server_.at(i)) break;

                    if (i == connected_to_server_.size() - 1)
                    {
                        connected_to_server_.push_back(connected_peer);
                        br = true;
                    }
                }

                if (br) break;

                if (connected_to_server_.empty())
                {
                    connected_to_server_.push_back(connected_peer);
                    break;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
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

        bool is_connected_to_server(std::string ip_s);
        bool has_connected_client(std::string ip_s);

        static std::vector<std::string> connected_to_server_;
        static std::vector<std::string> client_calls_;
    };
}