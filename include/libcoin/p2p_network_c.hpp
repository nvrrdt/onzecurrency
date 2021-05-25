#pragma once

#include <stdio.h>
#include <string.h>
#include <enet/enet.h>

#include "p2p_network.hpp"

using namespace Crowd;

namespace Coin
{
    class P2pNetworkC : public P2pNetwork
    {
    public:
        P2pNetworkC() : P2pNetwork(){};
        void start_coin();
    private:
    };
}