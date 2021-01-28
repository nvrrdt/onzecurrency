#include <iostream>
#include <fstream>
#include <iomanip>

#include "json.hpp"
#include "p2p.hpp"
#include "poco.hpp"
#include "ip_peers.hpp"
#include "crypto.hpp"
#include "base58.h"
#include "merkle_tree.hpp"

#include <vector>

#include <future>
#include <thread>

using namespace Crowd;

// https://stackoverflow.com/questions/19856665/how-to-use-vector-of-objects-in-different-files
std::vector<std::string> CreateBlock::list_of_new_peers_;

bool P2p::start_p2p(std::map<std::string, std::string> cred)
{
    // get ip from ip_peers.json // TODO: put this in p2p.hpp, it's a copy
    IpPeers ip;
    std::vector<std::string> ip_s = ip.get_ip_s();
    nlohmann::json json;
    to_json(json, ip_s);
    std::cout << "ip_s_client: " << json["ip_list"].size() << std::endl;

    const std::string ip_mother_peer = json["ip_list"][0]; // TODO: ip should later be randomly taken from rocksdb and/or a pre-defined list

    if (json["ip_list"].size() == 1) // 1 ip == ip_mother_peer
    {
        Tcp t;
        Crypto c;
        nlohmann::json message_j, to_sign_j, transaction_j, list_of_transactions_j, rocksdb_j;
        message_j["req"] = "intro_peer";
        message_j["hash_of_email"] = cred["email_hashed"]; // = id requester
        message_j["salt_of_req"] = cred["salt"]; // TODO: is the salt and the full_hash done in liblogin?
        message_j["full_hash"] = cred["full_hash"]; // TODO control fullhash
        message_j["email_of_req"] = cred["email"];
        
        message_j["pub_key"] = c.get_pub_key();

        to_sign_j["pub_key"] = message_j["pub_key"];
        to_sign_j["full_hash"] = message_j["full_hash"];

        auto [signature, succ] = c.sign(to_sign_j.dump());
        if (succ)
        {
            message_j["signature"] = base58::EncodeBase58(signature);
        }

        t.client("", ip_mother_peer, "hash_of_mother_peer", message_j.dump(), "pub_key"); // mother server must respond with ip_peer and ip_upnp_peer

        if (t.get_tcp_closed_client())
        {
            std::cout << "Connection was closed, probably no server reachable!" << std::endl;
            // you are the only peer and can create a block
            // + timestamp for the block
            std::cout << "dump: " << message_j.dump() << std::endl; // test this first ...

            std::shared_ptr<std::stack<std::string>> s_shptr = make_shared<std::stack<std::string>>();
            s_shptr->push(to_sign_j.dump());
            merkle_tree mt;
            s_shptr = mt.calculate_root_hash(s_shptr);
            transaction_j["full_hash"] = message_j["full_hash"];
            transaction_j["pub_key"] = message_j["pub_key"];
            list_of_transactions_j.push_back(transaction_j);
            std::string datetime = mt.time_now();
            std::cout << "jep: " << list_of_transactions_j.dump() << std::endl;
            mt.create_block(datetime, s_shptr->top(), list_of_transactions_j);

            // Update rocksdb
            Poco p;
            rocksdb_j["version"] = "O.1";
            rocksdb_j["ip"] = ip_mother_peer;
            rocksdb_j["server"] = true;
            rocksdb_j["fullnode"] = true;
            rocksdb_j["hash_email"] = message_j["hash_of_email"];
            rocksdb_j["salt"] = message_j["salt_of_req"];
            rocksdb_j["block"] = 0;
            rocksdb_j["pub_key"] = message_j["pub_key"];
            std::string hash_email = rocksdb_j["hash_email"];
            std::string salt = rocksdb_j["salt"];
            std::string fullhash =  c.create_base58_hash(hash_email + salt);
            p.Put(fullhash, rocksdb_j.dump());
            std::cout << "zijn we hier? " << std::endl;
        }
        else
        {
            std::cout << "elseelse" << std::endl;
            // you are not alone and your ip_list, blockchain, rochksdb must be updated, you must connect a peer's ip you get from mother_peer
        }
        return true;
    }

    // get ip_peer from mother_peer
    Tcp t;
    nlohmann::json message_j;
    message_j["req"] = "ip_peer";
    message_j["hash_of_req"] = cred["email_hashed"];
    nlohmann::json response = nlohmann::json::parse(t.client("", ip_mother_peer, "hash_of_mother_peer", message_j.dump(), "pub_key")); // mother server must respond with ip_peer and ip_upnp_peer

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
    if (t.server() == 0) { // wait 5 seconds, mother_peer tries to connect
        if (response["ip_upnp_peer"] != "") {
            t.client(response["ip_upnp_peer"], response["ip_peer"], response["hash_peer"], "server", "pub_key"); // server must update all peers with my ip, my id, my server being
        } else {        
            t.client("", response["ip_peer"], response["hash_peer"], "server", "pub_key"); // server must update all peers with my ip, my id, my server being
        }

        std::packaged_task<void()> task1([] {
            Tcp t;
            t.server();
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
            t.server();
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

        std::string email_hashed = cred["email_hashed"];
        std::packaged_task<void()> task1([email_hashed] {
            Poco p;
            std::string upnp_peer = p.FindUpnpPeer(email_hashed);
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