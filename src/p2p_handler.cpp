// this one manages p2p behaviour
// always listening server
// unless polling the network as a client

#include <iostream>
#include <fstream>
#include <iomanip>
#include "json.hpp"

#include "p2p_handler.hpp"
#include "ping.hpp"

#include <future>
#include <thread>

using namespace crowd;

void p2p_handler::p2p_switch(string task_client)
{
    p2p_handler peers;
    vector<string> ip_list = peers.parse_ip_peers_json();

    string online_ip;

    for (string& ip_adress : ip_list)
    {
        system_ping sp;
        if (sp.test_connection(ip_adress, 1)) // ping ip_adress to see if it is online
        {
            online_ip = ip_adress;
            break;
        }
    }

    p2p_handler cl;
    string response = cl.client(online_ip, task_client);

    // TODO: save response to a file
    p2p_handler::save_blockchain(response);

    p2p_handler se;
    se.server_main();
}

vector<string> p2p_handler::parse_ip_peers_json() // https://github.com/nlohmann/json
{
    ifstream file("../ip_peers.json");
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

void p2p_handler::save_blockchain(string response)
{
    // create the block on disk
    ofstream ofile("../blockchain/block0000000000.json", ios::out | ios::trunc);

    ofile << response;
    ofile.close();
}