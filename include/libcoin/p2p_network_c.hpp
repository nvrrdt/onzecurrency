#pragma once

#include <stdio.h>
#include <string.h>
#include <enet/enet.h>

#include "p2p_network.hpp"

using namespace Crowd;

namespace Coin
{
    class P2pNetworkC
    {
    public:
        void start_coin();
        void handle_read_server_c(nlohmann::json buf_j);
        void handle_read_client_c(nlohmann::json buf_j);
    private:
        void intro_tx(nlohmann::json buf_j);
        void new_tx(nlohmann::json buf_j);
    private:
    };
}