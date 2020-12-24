#include <iostream>
#include <fstream>
#include <iomanip>
#include "json.hpp"

#include "p2p.hpp"

#include <future>
#include <thread>

using namespace Crowd;

// TODO TODAY:
// Procedure: start in client, connect ip_mother_peer, recv or not recv for continue server
// if not then try upnp, if not then client, ask mother my_upnp_peer

// update the blockchain too, update rocksdb too

bool P2p::StartP2p()
{
    const std::string ip_mother_peer = "13.58.174.105"; // TODO: should be later taken from rocketdb or a pre-defined list

    // be a client to the mother server, if a reaction then you are a server
    Udp u;
    nlohmann::json response = u.udp_client(ip_mother_peer, "", "my_id"/*, "pubKey"*/);

    // response["ip_upnp_peer"] is "" of een ip, om naar H(my_id) alles te downloaden

    if (response["message"] == "true") {
        u.udp_server();
    } else {
        // try upnp
    };

    if (response["ip_upnp_peer"] != "") {
        u.udp_client(response["ip_upnp_peer"], response["ip_peer"], "update");
    }

    return true;
}




//bool P2p::IsUpnp()
//{
    /**
     * check upnp
     * if upnp = udp_server in thread
     * if not upnp = udp_client in thread
     * do some tests with timeout
     */
/*
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

        return true;
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

        return false;
    }
    
}*/

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