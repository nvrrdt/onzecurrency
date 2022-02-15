
#include "main.hpp"

#include <map>

#include "auth.hpp"
#include "p2p.hpp"
#include "json.hpp"
#include "p2p_network.hpp"
#include "p2p_network_c.hpp"

#include "print_or_log.hpp"
#include "configdir.hpp"

extern int USE_LOG;

using namespace Crowd;
using namespace Coin;

int main(int argc, char *argv[])
{
    ConfigDir cd;
    cd.CreateDirInConfigDir("log");
    Common::Print_or_log pl;
    pl.init();

    Auth a;
    std::map<std::string, std::string> cred = a.authentication("terminal");

    if (cred["error"] == "true")
    {
        pl.handle_print_or_log({"Error with authenticating"});
                
        return 1;
    }
    else
    {
        // start crowd
        std::packaged_task<void()> task1([cred] {
            P2p p;
            p.start_crowd(cred);
        });
        // Run task on new thread.
        std::thread t1(std::move(task1));

        // start coin
        std::packaged_task<void()> task2([cred] {
            // P2pNetworkC pnc;
            // pnc.start_coin();
        });
        // Run task on new thread.
        std::thread t2(std::move(task2));

        // start server
        std::packaged_task<void()> task3([] {
            P2pNetwork pn;
            pn.p2p_server();
        });
        // Run task on new thread.
        std::thread t3(std::move(task3));

        t1.join();
        t2.join();
        t3.join();
       
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