#include <string>
#include <map>

#include "auth.hpp"
#include "p2p.hpp"
#include "json.hpp"
#include "p2p_network.hpp"

#include "main.hpp"

using namespace Crowd;

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
            p.start_p2p(cred);
        });
        // Run task on new thread.
        std::thread t1(std::move(task1));

        // input to create a transaction (tx)
        while (true)
        {
            std::string to_full_hash = "", amount = "";
            std::cout << "Tx to: ";
            std::cin >> to_full_hash;

            if (to_full_hash == "q")
            {
                // TODO send an intro_offline request here

                P2pNetwork pn;
                pn.set_quit_server_req(true);
                break;
            }

            std::cout << "Amount: ";
            std::cin >> amount;

            std::cout << std::endl;
        }

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