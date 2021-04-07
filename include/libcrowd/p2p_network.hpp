#pragma once

#include <stdio.h>
#include <string.h>
#include <enet/enet.h>

#include <iostream>
#include <string>
#include <vector>

#include "p2p_message.hpp"
#include "json.hpp"

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
        void do_read_header();
        void do_read_body();
        void handle_read();
        void set_resp_msg(std::string msg);
        void get_sleep_and_create_block();
        void set_closed_client(std::string closed)
        {
            closed_client_ = closed;
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

        std::string closed_client_;
    };
}