#include "p2p_network.hpp"

#include <unistd.h>

using namespace Crowd;

int P2pNetwork::p2p_client(char *ip)
{
    int connected=0;

    if (enet_initialize() != 0)
    {
        printf("Could not initialize enet.\n");
        return 0;
    }

    client_ = enet_host_create(NULL, 1, 2, 5760/8, 1440/8);

    if (client_ == NULL)
    {
        printf("Could not create client.\n");
        return 0;
    }

    enet_address_set_host(&address_, ip);
    address_.port = PORT;

    peer_ = enet_host_connect(client_, &address_, 2, 0);

    if (peer_ == NULL)
    {
        printf("Could not connect to server\n");
        return 0;
    }

    if (enet_host_service(client_, &event_, 1000) > 0 &&
        event_.type == ENET_EVENT_TYPE_CONNECT)
        {

        printf("Connection to %s succeeded.\n", ip);
        connected++;

        strncpy(buffer_, "hello", 512);
        packet_ = enet_packet_create(buffer_, strlen(buffer_)+1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer_, 0, packet_);

    }
    else
    {
        enet_peer_reset(peer_);
        printf("Could not connect to %s.\n", ip);
        return 0;
    }

    while (1)
    {
        while (enet_host_service(client_, &event_, 1000) > 0)
        {
            switch (event_.type)
            {
                case ENET_EVENT_TYPE_RECEIVE:
                    puts( (char*) event_.packet->data);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    connected=0;
                    printf("You have been disconnected.\n");
                    return 2;
            }
        }

        if (connected)
        {
            strncpy(buffer_, "buffer___", 512);

            if (strlen(buffer_) == 0) { continue; }

            if (strncmp("q", buffer_, 512) == 0)
            {
                connected=0;
                enet_peer_disconnect(peer_, 0);
                continue;
            } 

            packet_ = enet_packet_create(buffer_, strlen(buffer_)+1, ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(peer_, 0, packet_);
        }
    }

    enet_deinitialize();
}