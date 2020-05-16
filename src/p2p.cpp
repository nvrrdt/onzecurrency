// this one manages p2p behaviour
// always listening server
// unless polling the network as a client

#include <iostream>
#include <fstream>
#include <iomanip>
#include "json.hpp"

#include "p2p.hpp"

#include <future>
#include <thread>

using namespace Crowd;

P2p::P2p()
{
    /**
     * check upnp
     * if upnp = udp_server in thread
     * if not upnp = udp_client in thread
     * do some tests with timeout
     */

    Upnp u;
    if (u.Upnp_main() == 0)
    {
        // Upnp possible
        std::packaged_task<void()> task1([] {
            Udp u;
            u.udp_server();
        });
        
        // Run task on new thread.
        std::thread t1(std::move(task1));

        t1.join();
    }
    else
    {
        // Upnp NOT possible
        std::packaged_task<void()> task2([] {
            Udp u;
            u.udp_client();
        });
        
        // Run task on new thread.
        std::thread t2(std::move(task2));

        t2.join();
    }
    
}

/*
vector<string> P2p::parse_ip_adress_master_peer_json() // https://github.com/nlohmann/json
{
    ifstream file("./ip_adress_master_peer.json");
	nlohmann::json j;
	
	file >> j;

	if (j.is_object()) {
        vector<string> ip_list;

        for (auto& element : j["ip_list"])
        {
            ip_list.push_back(element);
        }

        return ip_list;
	}
}

void P2p::save_blockchain(string response)
{
    // create the block on disk
    ofstream ofile("./blockchain/block0000000000.json", ios::out | ios::trunc);

    ofile << response;
    ofile.close();
}
*/