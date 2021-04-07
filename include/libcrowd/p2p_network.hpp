#pragma once

#include <stdio.h>
#include <string.h>
#include <enet/enet.h>
#include "p2p_message.hpp"

// This is the port for the p2p_server and p2p_client
#define PORT (1975)

namespace Crowd
{
    class P2pNetwork
    {
    public:
        int p2p_server();
        int p2p_client(char *ip);
    private:
        char buffer_[512];

        ENetHost  *client_;
        ENetHost   *server_;
        ENetAddress  address_;
        ENetEvent  event_;
        ENetPeer  *peer_;
        ENetPacket  *packet_;
    };
}