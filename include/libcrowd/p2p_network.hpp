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
#include "message_vec.hpp"
#include "poco_crowd.hpp"
#include "poco_coin.hpp"
#include "merkle_tree.hpp"
#include "all_hashes.hpp"

// This is the port for the p2p_server and p2p_client
#define PORT (1975)

namespace Crowd
{
    class P2pNetwork
    {
    public:
        int p2p_server();
        int p2p_client(std::string ip_s, std::string message);
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
        void do_read_header_server();
        void do_read_body_server();
        void do_read_header_client();
        void do_read_body_client();
        void handle_read_server();
        void handle_read_client();
        void get_sleep_and_create_block_server();
        void get_sleep_and_create_block_client();
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
        void update_your_blocks(nlohmann::json buf_j);
        void update_your_rocksdb(nlohmann::json buf_j);
        void intro_block(nlohmann::json buf_j);
        void new_block(nlohmann::json buf_j);
        void your_full_hash(nlohmann::json buf_j);
        void hash_comparison(nlohmann::json buf_j);
        void update_my_blocks_and_rocksdb(nlohmann::json buf_j);
        void intro_online(nlohmann::json buf_j);
        void new_online(nlohmann::json buf_j);
        
        static bool get_quit_server_req()
        {
            return quit_server_;
        }
    protected:
        ENetHost   *server_;
    private:
        char buffer_[p2p_message::max_body_length];

        ENetHost  *client_;
        ENetAddress  address_;
        ENetEvent  event_;
        ENetPeer  *peer_;
        ENetPacket  *packet_;

        p2p_message read_msg_;
        p2p_message resp_msg_;
        std::string buf_client_;
        std::string buf_server_;
        Poco::MessageVec message_j_vec_; // maybe std::shared_ptr<MessageVec> message_j_vec_ = std::make_shared<MessageVec>() ?? compare with header!!
        Poco::AllHashes all_hashes_;

        static std::string closed_client_;
        static uint32_t ip_new_co_;

        static bool quit_server_;
    };
}