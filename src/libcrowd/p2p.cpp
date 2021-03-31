#include <iostream>
#include <fstream>
#include <iomanip>

#include "json.hpp"
#include "p2p.hpp"
#include "rocksy.hpp"
#include "crypto.hpp"
#include "merkle_tree.hpp"
#include "auth.hpp"

#include <vector>

#include <future>
#include <thread>

using namespace Crowd;

std::string Auth::my_full_hash_;

bool P2p::start_p2p(std::map<std::string, std::string> cred)
{
    if (cred["new_peer"] == "true")
    {
        Rocksy* rocksy = new Rocksy();
        if (rocksy->TotalAmountOfPeers() <= 1)
        {
            delete rocksy;
            Tcp t;
            Crypto crypto;
            Protocol proto;
            nlohmann::json message_j, to_sign_j, to_block_j, entry_tx_j, entry_transactions_j, exit_tx_j, exit_transactions_j, rocksdb_j;

            message_j.empty();
            to_sign_j.empty();
            to_block_j.empty();
            entry_tx_j.empty();
            entry_transactions_j.empty();
            exit_tx_j.empty();
            exit_transactions_j.empty();
            rocksdb_j.empty();
            
            message_j["req"] = "intro_peer";
            message_j["email_of_req"] = cred["email"];
            message_j["hash_of_email"] = cred["email_hashed"]; // = id requester
            message_j["prev_hash_of_req"] = cred["prev_hash"];
            message_j["full_hash_co"] = "0"; // TODO proto.get_last_prev_hash()
            message_j["latest_block"] = proto.get_last_block_nr();
            
            message_j["ecdsa_pub_key"] = cred["ecdsa_pub_key"];
            message_j["rsa_pub_key"] = cred["rsa_pub_key"];

            to_sign_j["ecdsa_pub_key"] = cred["ecdsa_pub_key"];
            to_sign_j["rsa_pub_key"] = cred["rsa_pub_key"];
            to_sign_j["email"] = cred["email"];
            std::string to_sign_s = to_sign_j.dump();
            // std::cout << "to_sign_s: " << to_sign_s << std::endl;
            ECDSA<ECP, SHA256>::PrivateKey private_key;
            std::string signature;
            crypto.ecdsa_load_private_key_from_string(private_key);
            if (crypto.ecdsa_sign_message(private_key, to_sign_s, signature))
            {
                message_j["signature"] = crypto.base64_encode(signature);

                // std::cout << "verification0p: " << std::endl;
                // std::cout << "signature_bin: " << "X" << signature << "X" << std::endl;
                // std::cout << "base64_signature: " << "X" << crypto.base64_encode(signature) << "X" << std::endl;
            }

            // begin test

            // ECDSA<ECP, SHA256>::PublicKey public_key_ecdsa;
            // std::string ecdsa_pub_key_s = cred["ecdsa_pub_key"];
            // crypto.ecdsa_string_to_public_key(ecdsa_pub_key_s, public_key_ecdsa);
            // std::string signature2 = message_j["signature"];
            // std::string signature3 = crypto.base64_decode(signature2);

            // if (crypto.ecdsa_verify_message(public_key_ecdsa, to_sign_s, signature3))
            // {
            //     std::cout << "verification1p: " << std::endl;
            //     std::cout << "ecdsa_p_key: " << "X" << ecdsa_pub_key_s << "X" << std::endl;
            //     std::cout << "to_sign_s: " << "X" << to_sign_s << "X" << std::endl;
            //     std::cout << "signature_bin: " << "X" << signature3 << "X" << std::endl;
            //     std::cout << "base64_signature: " << "X" << signature2 << "X" << std::endl;
            // }
            // else
            // {
            //     std::cout << "verification2p: " << std::endl;
            //     std::cout << "ecdsa_p_key: " << "X" << ecdsa_pub_key_s << "X" << std::endl;
            //     std::cout << "to_sign_s: " << "X" << to_sign_s << "X" << std::endl;
            //     std::cout << "signature_bin: " << "X" << signature3 << "X" << std::endl;
            //     std::cout << "hex_signature: " << "X" << signature2 << "X" << std::endl;
            // }

            // end test

            std::string srv_ip = "";
            std::string ip_mother_peer = "51.158.68.232"; // TODO: ip should later be randomly taken from rocksdb and/or a pre-defined list
            message_j["ip"] = ip_mother_peer;
            std::string peer_hash = "";

            rocksdb_j["version"] = "O.1";
            rocksdb_j["ip"] = ip_mother_peer;
            rocksdb_j["server"] = true;
            rocksdb_j["fullnode"] = true;
            rocksdb_j["hash_email"] = message_j["hash_of_email"];
            rocksdb_j["prev_hash"] = cred["prev_hash"];
            std::string hash_email = cred["email_hashed"];
            std::string prev_hash = cred["prev_hash"];
            std::string email_prev_hash_app = hash_email + prev_hash;
            std::string full_hash = crypto.bech32_encode_sha256(email_prev_hash_app);
            rocksdb_j["full_hash"] = full_hash;
            rocksdb_j["block_nr"] = proto.get_last_block_nr();
            std::cout << "get last block number from the first block: " << rocksdb_j["block_nr"] << std::endl;
            rocksdb_j["ecdsa_pub_key"] = message_j["ecdsa_pub_key"];
            rocksdb_j["rsa_pub_key"] = message_j["rsa_pub_key"];

            message_j["rocksdb"] = rocksdb_j;
            std::string message = message_j.dump();
            t.client(srv_ip, ip_mother_peer, peer_hash, message); // mother server must respond with ip_peer and ip_server_peer

            if (t.get_tcp_closed_client())
            {
                std::cout << "Connection was closed, probably no server reachable!" << std::endl;

                // you are the only peer and can create a block

                to_block_j["full_hash"] = full_hash;
                to_block_j["ecdsa_pub_key"] = cred["ecdsa_pub_key"];
                to_block_j["rsa_pub_key"] = cred["rsa_pub_key"];

                std::shared_ptr<std::stack<std::string>> s_shptr = make_shared<std::stack<std::string>>();
                s_shptr->push(to_block_j.dump());
                merkle_tree mt;
                s_shptr = mt.calculate_root_hash(s_shptr);
                entry_tx_j["full_hash"] = to_block_j["full_hash"];
                entry_tx_j["ecdsa_pub_key"] = to_block_j["ecdsa_pub_key"];
                entry_tx_j["rsa_pub_key"] = to_block_j["rsa_pub_key"];
                entry_transactions_j.push_back(entry_tx_j);
                exit_tx_j["full_hash"] = "";
                exit_transactions_j.push_back(exit_tx_j);
                std::string datetime = mt.time_now();
                std::string root_hash_data = s_shptr->top();
std::cout << "root_hash_data: " << root_hash_data << std::endl;
                nlohmann::json block_j = mt.create_block(datetime, root_hash_data, entry_transactions_j, exit_transactions_j);
                std::string block_nr = proto.get_last_block_nr();
                mt.save_block_to_file(block_j, block_nr);

                // Update rocksdb
                std::string rocksdb_s = rocksdb_j.dump();
                Rocksy* rocksy = new Rocksy();
                rocksy->Put(full_hash, rocksdb_s);
                delete rocksy;
                std::cout << "Rocksdb updated and server started! " << std::endl;

                std::packaged_task<void()> task1([] {
                    Tcp t;
                    t.server();
                });
                // Run task on new thread.
                std::thread t1(std::move(task1));
                t1.join();
            }
            else if (t.get_ip_new_co() != "")
            {
                std::string ip = t.get_ip_new_co();
                t.client(srv_ip, ip, peer_hash, message);
            }
            else
            {
                std::cout << "The t.client did it's job succesfully and you should be added in the blockchain!" << std::endl;

                std::packaged_task<void()> task1([] {
                    Tcp t;
                    t.server();
                });
                // Run task on new thread.
                std::thread t1(std::move(task1));
                t1.join();
            }
            return true;
        }
        else
        {
            delete rocksy;
            // 2 or more peers ...
            // get ip of online peer in rocksdb
            // then t.client to that peer
            // and that peer must create the block

            // TODO: this is only applicable when rocksdb is already filled with pears
            // which is rather rarely the case when a new user enters the network
            // so this code can be postponed to later
        }
    }
    else if (cred["new_peer"] == "false")
    {
        // existing user
        // t.client with {"online": "true"}
    }
    else
    {
        // something wrong cred["new_peer"] not present or correct
        std::cerr << "Wrong cred[\"new_peer\"]" << std::endl;
    }

    // // get ip_peer from mother_peer
    // Tcp t;
    // nlohmann::json message_j;
    // message_j["req"] = "ip_peer";
    // message_j["hash_of_req"] = cred["email_hashed"];
    // nlohmann::json response = nlohmann::json::parse(t.client("", ip_mother_peer, "hash_of_mother_peer", message_j.dump(), "pub_key")); // mother server must respond with ip_peer and ip_server_peer

    // // update your blockchain and rocksdb
    // if (response["ip_peer"] != "") {
    //     if (response["ip_server_peer"] != "") {
    //         t.client(response["ip_server_peer"], response["ip_peer"], response["hash_peer"], "update", "pub_key"); // server must respond with packets updating rocksdb and blockchain
    //     } else {
    //         t.client("", response["ip_peer"], response["hash_peer"], "update", "pub_key"); // server must respond with packets updating rocksdb and blockchain
    //     }
    // } else {
    //     return false;
    // }
    
    // // prepare for becoming a peer and update the rocksdb of all peers with my presence
    // if (t.server() == 0) { // wait 5 seconds, mother_peer tries to connect
    //     if (response["ip_server_peer"] != "") {
    //         t.client(response["ip_server_peer"], response["ip_peer"], response["hash_peer"], "server", "pub_key"); // server must update all peers with my ip, my id, my server being
    //     } else {        
    //         t.client("", response["ip_peer"], response["hash_peer"], "server", "pub_key"); // server must update all peers with my ip, my id, my server being
    //     }

    //     std::packaged_task<void()> task1([] {
    //         Tcp t;
    //         t.server();
    //     });
    //     // Run task on new thread.
    //     std::thread t1(std::move(task1));
    //     t1.join();
    // } else if (Upnp u; u.Upnp_main() == 0) { // try upnp to become a server or else
    //     if (response["ip_server_peer"] != "") {
    //         t.client(response["ip_server_peer"], response["ip_peer"], response["hash_peer"], "server", "pub_key"); // server must update all peers with my ip, my id, my server being
    //     } else {
    //         t.client("", response["ip_peer"], response["hash_peer"], "server", "pub_key"); // server must update all peers with my ip, my id, my server being
    //     }

    //     std::packaged_task<void()> task1([] {
    //         Tcp t;
    //         t.server();
    //     });
    //     // Run task on new thread.
    //     std::thread t1(std::move(task1));
    //     t1.join();
    // } else {
    //     if (response["ip_server_peer"] != "") {
    //         t.client(response["ip_server_peer"], response["ip_peer"], response["hash_peer"], "client", "pub_key"); // server must update all peers with my ip, my id, my client being
    //     } else {
    //         t.client("", response["ip_peer"], response["hash_peer"], "client", "pub_key"); // server must update all peers with my ip, my id, my client being
    //     }

    //     std::string email_hashed = cred["email_hashed"];
    //     std::packaged_task<void()> task1([email_hashed] {
    //         Rocksy r;
    //         std::string server_peer = p.FindServerPeer(email_hashed);
    //         Tcp t;
    //         t.client(server_peer, "", "", "register", "pub_key"); // server should keep connection open to be able to communicate
    //     });
    //     // Run task on new thread.
    //     std::thread t1(std::move(task1));
    //     t1.join();
    // }
     
    return true;
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