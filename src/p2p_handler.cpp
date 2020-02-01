// this one manages p2p behaviour
// always listening server
// unless polling the network as a client

#include <iostream>
#include <fstream>
#include <iomanip>
#include "json.hpp"

#include "p2p_handler.hpp"
  
using namespace crowd;

void p2p_handler::p2p_switch()
{
    p2p_handler peers;
    vector<string> ip_list = peers.parse_ip_peers_json();

    for (string& ip_adress : ip_list)
    {
        p2p_handler cl;
        cl.client(ip_adress);
    }

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
