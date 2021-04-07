#include "p2p_network.hpp"

using namespace Crowd;

int P2pNetwork::p2p_server()
{
    int  i;

    if (enet_initialize() != 0)
    {
        printf("Could not initialize enet.");
        return 0;
    }

    address_.host = ENET_HOST_ANY;
    address_.port = PORT;

    server_ = enet_host_create(&address_, 100, 2, 0, 0);

    if (server_ == NULL)
    {
        printf("Could not start server.\n");
        return 0;
    }
    while (1)
    {
        while (enet_host_service(server_, &event_, 1000) > 0)
        {
            switch (event_.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    if (event_.peer->data == NULL)
                    {
                        event_.peer->data = 
                        malloc(strlen((char*) event_.packet->data)+1);
                        strcpy((char*) event_.peer->data, (char*) event_.packet->data);
                        sprintf(buffer_, "%s has connected\n", (char*) event_.packet->data);
                        packet_ = enet_packet_create(buffer_, strlen(buffer_)+1, 0);
                        enet_host_broadcast(server_, 1, packet_);
                        enet_host_flush(server_);
                    }
                    else
                    {
                        for (int i = 0; i < server_->peerCount; i++)
                        {
                            if (&server_->peers[i] != event_.peer)
                            {
                                sprintf(buffer_, "%s: %s", (char*) event_.peer->data, (char*) event_.packet->data);
                                packet_ = enet_packet_create(buffer_, strlen(buffer_)+1, 0);
                                enet_peer_send(&server_->peers[i], 0, packet_);
                                enet_host_flush(server_);
                            }
                            else
                            {

                            }
                        }
                    }
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    sprintf(buffer_, "%s has disconnected.", (char*) event_.peer->data);
                    packet_ = enet_packet_create(buffer_, strlen(buffer_)+1, 0);
                    enet_host_broadcast(server_, 1, packet_);
                    free(event_.peer->data);
                    event_.peer->data = NULL;
                    break;

                default:
                    printf("Tick tock.\n");
                    break;
            }

        }
    }

    enet_host_destroy(server_);
    enet_deinitialize();
}