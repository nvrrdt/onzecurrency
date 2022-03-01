#pragma once

#include <stdio.h>
#include <string.h>

#include "p2p_network.hpp"

using namespace Crowd;

namespace Coin
{
    class P2pC
    {
    public:
        void start_coin();
    private:
        bool validate_full_hash(std::string to_full_hash);
        bool validate_amount(std::string amount);
    };

    class P2pNetworkC
    {
    public:
        void handle_read_client_c(nlohmann::json buf_j);
        void handle_read_server_c(nlohmann::json buf_j, tcp::socket socket);
    private:
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
        void intro_online_c(nlohmann::json buf_j, tcp::socket socket);
        void new_online_c(nlohmann::json buf_j, tcp::socket socket);
    private:
    };

    class P2pSessionC: public P2pSession
    {
    public:
        P2pSessionC(tcp::socket socket): P2pSession(std::move(socket)) {}
    };
}