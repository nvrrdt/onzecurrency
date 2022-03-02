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

        static void set_coin_update_complete(bool completed)
        {
            coin_update_complete_ = completed;
        }
    private:
        static bool get_coin_update_complete()
        {
            return coin_update_complete_;
        }
    private:
        bool validate_full_hash(std::string to_full_hash);
        bool validate_amount(std::string amount);
        static bool coin_update_complete_;
    };

    class P2pNetworkC
    {
    public:
        void handle_read_client_c(nlohmann::json buf_j, boost::asio::io_context &io_context, const tcp::resolver::results_type &endpoints);
        void handle_read_server_c(nlohmann::json buf_j, tcp::socket socket);
    private:
        // client
        void update_me_c_client(nlohmann::json buf_j, boost::asio::io_context &io_context, const tcp::resolver::results_type &endpoints);
        void update_you_c_client(nlohmann::json buf_j, boost::asio::io_context &io_context, const tcp::resolver::results_type &endpoints);
    private:
        // server
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
        void update_you_c(nlohmann::json buf_j, tcp::socket socket);
    private:
    };

    class P2pClientC: public P2pClient
    {
    public:
        P2pClientC(boost::asio::io_context &io_context, const tcp::resolver::results_type &endpoints)
            : P2pClient(std::ref(io_context), std::ref(endpoints)) {}
    };

    class P2pSessionC: public P2pSession
    {
    public:
        P2pSessionC(tcp::socket socket): P2pSession(std::move(socket)) {}
    };
}