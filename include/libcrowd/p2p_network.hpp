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
#include "create_block.hpp"
#include "merkle_tree.hpp"

// This is the port for the p2p_server and p2p_client
#define PORT (1975)

namespace Crowd
{
    class P2pNetwork
    {
    public:
        int p2p_server();
        int p2p_client(std::string ip_s, std::string message);
        std::string get_closed_client()
        {
            return closed_client_;
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
        void set_resp_your_hash_server(ENetPeer *participant, std::string msg);
        void get_sleep_and_create_block_server();
        void get_sleep_and_create_block_client();
        void set_closed_client(std::string closed)
        {
            closed_client_ = closed;
        }
        void add_to_all_full_hashes(ENetPeer *participant, std::string full_hash_req)
        {
            all_full_hashes_[participant] = full_hash_req;
        }

        std::map<ENetPeer*, std::string> get_all_full_hashes()
        {
            return all_full_hashes_;
        }

        void reset_all_full_hashes()
        {
            all_full_hashes_.clear();
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
        std::string buf_;
        MessageVec message_j_vec_;
        std::map<ENetPeer*, std::string> all_full_hashes_;

        std::string closed_client_;
    };
}