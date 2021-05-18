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
#include "poco.hpp"
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
    private:
        std::vector<std::string> split(const std::string& str, int splitLength);
        void do_read_header_server();
        void do_read_body_server();
        void do_read_header_client();
        void do_read_body_client();
        void handle_read_server();
        void handle_read_client();
        void set_resp_msg_server(std::string msg);
        void set_resp_msg_client(std::string msg);
        void set_resp_your_hash_server(enet_uint32 participant, std::string msg);
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
        
    private:
        char buffer_[p2p_message::max_body_length];

        ENetHost  *client_;
        ENetHost   *server_;
        ENetAddress  address_;
        ENetEvent  event_;
        ENetPeer  *peer_;
        ENetPacket  *packet_;

        p2p_message read_msg_;
        p2p_message resp_msg_;
        std::string buf_client_;
        std::string buf_server_;
        MessageVec message_j_vec_; // maybe std::shared_ptr<MessageVec> message_j_vec_ = std::make_shared<MessageVec>() ?? compare with header!!
        AllHashes all_hashes_;

        static std::string closed_client_;
        static uint32_t ip_new_co_;
    };
}