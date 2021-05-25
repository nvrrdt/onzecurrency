#include <string>
#include <map>

#include "auth.hpp"
#include "p2p.hpp"
#include "json.hpp"
#include "p2p_network.hpp"
#include "p2p_network_c.hpp"

#include "main.hpp"

using namespace Coin;

int main(int argc, char *argv[])
{
    Auth a;
    std::map<std::string, std::string> cred = a.authentication();

    if (cred["error"] == "true")
    {
        std::cerr << "Error with authenticating" << std::endl;
        return 1;
    } else {
        std::packaged_task<void()> task1([cred] {
            P2p p;
            p.start_crowd(cred);
        });
        // Run task on new thread.
        std::thread t1(std::move(task1));

        P2pNetworkC pnc;
        pnc.start_coin();

        t1.join();
       
    }

    // // example of how to enable upnp
    // std::cout << "server_peer: " << ip_mother_peer << ", my_ip: " << my_public_ip << std::endl;
    // if (ip_mother_peer == my_public_ip)
    // {
    //     Upnp upnp;
    //     Udp udp;
    //     if(upnp.Upnp_main() == 0) // upnp possible
    //     {
    //         udp.udp_server();
    //     }
    // }

    return 0;
}