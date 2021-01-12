#include <iostream>
#include <fstream>
#include <iomanip>

#include "json.hpp"
#include "p2p.hpp"
#include "poco.hpp"

#include <future>
#include <thread>

using namespace Crowd;

// struct message_contents {
//     std::string my_id;
//     std::string peer_id;
//     std::string server_peer_id;
//     std::string ip_peer;
//     std::string ip_server_peer;
//     std::string version;
//     bool fullnode;
//     std::string pub_key;
//     bool server;    
// } message;

bool P2p::StartP2p(std::string my_id)
{
    const std::string ip_mother_peer = "13.58.174.105"; // TODO: ip should later be taken from rocksdb or a pre-defined list

    // get ip_peer from mother_peer
    Tcp t;
    nlohmann::json response = nlohmann::json::parse(t.client("", ip_mother_peer, "hash_of_mother_peer", "ip_peer", "pub_key")); // mother server must respond with ip_peer and ip_upnp_peer

    // update your blockchain and rocksdb
    if (response["ip_peer"] != "") {
        if (response["ip_upnp_peer"] != "") {
            t.client(response["ip_upnp_peer"], response["ip_peer"], response["hash_peer"], "update", "pub_key"); // server must respond with packets updating rocksdb and blockchain
        } else {
            t.client("", response["ip_peer"], response["hash_peer"], "update", "pub_key"); // server must respond with packets updating rocksdb and blockchain
        }
    } else {
        return false;
    }
    
    // prepare for becoming a peer and update the rocksdb of all peers with my presence
    if (t.server("try") == 0) { // wait 5 seconds, mother_peer tries to connect
        if (response["ip_upnp_peer"] != "") {
            t.client(response["ip_upnp_peer"], response["ip_peer"], response["hash_peer"], "server", "pub_key"); // server must update all peers with my ip, my id, my server being
        } else {        
            t.client("", response["ip_peer"], response["hash_peer"], "server", "pub_key"); // server must update all peers with my ip, my id, my server being
        }

        std::packaged_task<void()> task1([] {
            Tcp t;
            t.server("");
        });
        // Run task on new thread.
        std::thread t1(std::move(task1));
        t1.join();
    } else if (Upnp u; u.Upnp_main() == 0) { // try upnp to become a server or else
        if (response["ip_upnp_peer"] != "") {
            t.client(response["ip_upnp_peer"], response["ip_peer"], response["hash_peer"], "server", "pub_key"); // server must update all peers with my ip, my id, my server being
        } else {
            t.client("", response["ip_peer"], response["hash_peer"], "server", "pub_key"); // server must update all peers with my ip, my id, my server being
        }

        std::packaged_task<void()> task1([] {
            Tcp t;
            t.server("");
        });
        // Run task on new thread.
        std::thread t1(std::move(task1));
        t1.join();
    } else {
        if (response["ip_upnp_peer"] != "") {
            t.client(response["ip_upnp_peer"], response["ip_peer"], response["hash_peer"], "client", "pub_key"); // server must update all peers with my ip, my id, my client being
        } else {
            t.client("", response["ip_peer"], response["hash_peer"], "client", "pub_key"); // server must update all peers with my ip, my id, my client being
        }

        std::packaged_task<void()> task1([my_id] {
            Poco p;
            std::string upnp_peer = p.FindUpnpPeer(my_id);
            Tcp t;
            t.client(upnp_peer, "", "", "register", "pub_key"); // server should keep connection open to be able to communicate
        });
        // Run task on new thread.
        std::thread t1(std::move(task1));
        t1.join();
    }
     
    return true;
}

std::string Tcp::remove_trailing_characters(std::string buf)
{
    // there are trailing chartacters that need to be trimmed,
    // working with an header for comunicating the length of the body is probably a solution
    std::string str_buf(buf);
    std::size_t found = str_buf.find_last_of("}");

    return str_buf.erase(found+1, sizeof(str_buf));
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