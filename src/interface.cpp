#include "json.hpp"

#include "interface.hpp"
#include "p2p.hpp"
#include "poco.hpp"

using namespace Crowd;

// - hello network!
// - verification of blockchain
// - new user in network
// - goodbye network

Interface::Interface(std::string& my_user_login_hash)
{
    /**
     * - search your upnp provider and say hello to the network
     */

    Poco poco;
    uint32_t upnp_peer_key = poco.FindUpnpPeer(static_cast<uint32_t>(std::stoul(my_user_login_hash)));
    nlohmann::json upnp_peer_value = nlohmann::json::parse(poco.Get(upnp_peer_key));

    Upnp upnp;
    Udp udp;
    if(upnp.Upnp_main() == 0) // upnp possible?
    {
        if (udp.udp_client(upnp_peer_value["ip"].dump(), "helloupnpenabled") == true)
        {
            // set as upnp server
        }
        else
        {
            // find next upnp provider and set as new upnp server
        }
        
    }
    else
    {
        if (udp.udp_client(upnp_peer_value["ip"].dump(), "helloupnpdisabled") == true)
        {
            // set as upnp client
        }
        else
        {
            // find next upnp provider and set as upnp client
        }
    }
    
}