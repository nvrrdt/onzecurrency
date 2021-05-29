#pragma once

#include <stdio.h>
#include <string.h>
#include <enet/enet.h>

#include "p2p_network.hpp"

using namespace Crowd;

namespace Coin
{
    class P2pNetworkC: P2pNetwork
    {
    public:
        void start_coin();
        void handle_read_server_c(nlohmann::json buf_j);
        void handle_read_client_c(nlohmann::json buf_j);
    private:
        void hello_tx(nlohmann::json buf_j);
        void intro_tx(nlohmann::json buf_j);
        void new_tx(nlohmann::json buf_j);
        bool validate_full_hash(std::string to_full_hash);
        bool validate_amount(std::string amount);
        void intro_block_c(nlohmann::json buf_j);
        void new_block_c(nlohmann::json buf_j);
    private:
    };
}