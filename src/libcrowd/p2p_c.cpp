#include "p2p_network.hpp"

using namespace Crowd;

std::vector<std::string> P2pNetwork::split(const std::string& str, int splitLength)
{
   int NumSubstrings = str.length() / splitLength;
   std::vector<std::string> ret;

   for (auto i = 0; i < NumSubstrings; i++)
   {
        ret.push_back(str.substr(i * splitLength, splitLength));
   }

   // If there are leftover characters, create a shorter item at the end.
   if (str.length() % splitLength != 0)
   {
        ret.push_back(str.substr(splitLength * NumSubstrings));
   }

   return ret;
}

int P2pNetwork::p2p_client(std::string ip_s, std::string message)
{
    const char *ip = ip_s.c_str();

    int connected=0;

    if (enet_initialize() != 0)
    {
        printf("Could not initialize enet.\n");
        return 0;
    }

    client_ = enet_host_create(NULL, 1, 2, 0, 0);

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

    if (enet_host_service(client_, &event_, 1000) > 0 && event_.type == ENET_EVENT_TYPE_CONNECT)
    {

        printf("Connection to %s succeeded.\n", ip);
        connected++;

        std::vector<std::string> splitted = split(message, p2p_message::max_body_length);
        for (int i = 0; i < splitted.size(); i++)
        {
            char s[p2p_message::max_body_length];
            strncpy(s, splitted[i].c_str(), sizeof(s));

            p2p_message msg;
            msg.body_length(std::strlen(s));
            std::memcpy(msg.body(), s, msg.body_length());
            i == splitted.size() - 1 ? msg.encode_header(1) : msg.encode_header(0); // 1 indicates end of message eom, TODO perhaps a set_eom_flag(true) instead of an int

            packet_ = enet_packet_create(msg.data(), strlen(msg.data())+1, ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(peer_, 0, packet_);
        }
    }
    else
    {
        set_closed_client("closed_conn");

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
            strncpy(buffer_, "buffer___", p2p_message::max_body_length);

            if (strlen(buffer_) == 0) { continue; }

            if (strncmp("q", buffer_, p2p_message::max_body_length) == 0)
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