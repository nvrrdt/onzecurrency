#include <iostream>
#include <fstream>
#include <iomanip>

#include "json.hpp"
#include "p2p.hpp"
#include "rocksy.hpp"
#include "crypto.hpp"
#include "merkle_tree.hpp"
#include "auth.hpp"
#include "verification.hpp"
#include "block_matrix.hpp"
#include "synchronisation.hpp"

#include <vector>

#include <future>
#include <thread>

#include "p2p_network.hpp"

#include "print_or_log.hpp"

using namespace Crowd;

bool P2p::start_crowd(std::map<std::string, std::string> cred)
{
    Common::Print_or_log pl;
    if (cred["new_peer"] == "true")
    {
        Rocksy* rocksy = new Rocksy("usersdbreadonly");
        if (rocksy->TotalAmountOfPeers() < 1)
        {
            delete rocksy;
            Common::Crypto crypto;
            Protocol proto;
            nlohmann::json message_j, to_sign_j, to_block_j, entry_tx_j, entry_transactions_j, exit_tx_j, exit_transactions_j, rocksdb_j;
            
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
            // pl.handle_print_or_log({"to_sign_s:", to_sign_s});
            ECDSA<ECP, SHA256>::PrivateKey private_key;
            std::string signature;
            crypto.ecdsa_load_private_key_from_string(private_key);
            if (crypto.ecdsa_sign_message(private_key, to_sign_s, signature))
            {
                message_j["signature"] = crypto.base64_encode(signature);

                // pl.handle_print_or_log({"verification0p:"});
                // pl.handle_print_or_log({"signature_bin:", signature});
                // pl.handle_print_or_log({"base64_signature:", std::to_string(crypto.base64_encode(signature))});
            }

            // begin test

            // ECDSA<ECP, SHA256>::PublicKey public_key_ecdsa;
            // std::string ecdsa_pub_key_s = cred["ecdsa_pub_key"];
            // crypto.ecdsa_string_to_public_key(ecdsa_pub_key_s, public_key_ecdsa);
            // std::string signature2 = message_j["signature"];
            // std::string signature3 = crypto.base64_decode(signature2);

            // if (crypto.ecdsa_verify_message(public_key_ecdsa, to_sign_s, signature3))
            // {
            //     pl.handle_print_or_log({"verification1p:"});
            //     pl.handle_print_or_log({"ecdsa_p_key:", ecdsa_pub_key_s});
            //     pl.handle_print_or_log({"to_sign_s:", to_sign_s});
            //     pl.handle_print_or_log({"signature_bin:", std::to_string(signature3)});
            //     pl.handle_print_or_log({"base64_signature:", std::to_string(signature2)});
            // }
            // else
            // {
            //     pl.handle_print_or_log({"verification2p:"});
            //     pl.handle_print_or_log({"ecdsa_p_key:", ecdsa_pub_key_s});
            //     pl.handle_print_or_log({"to_sign_s:", to_sign_s});
            //     pl.handle_print_or_log({"signature_bin:", std::to_string(signature3)});
            //     pl.handle_print_or_log({"hex_signature:", std::to_string(signature2)});
            // }

            // end test

            std::string ip_mother_peer = "212.47.231.236"; // TODO: ip should later be randomly taken from rocksdb and/or a pre-defined list
            uint32_t ip_mother_peer_number;
            ip_string_to_number(ip_mother_peer.c_str(), ip_mother_peer_number);

            message_j["ip"] = ip_mother_peer_number;

            rocksdb_j["version"] = "O.1";
            rocksdb_j["ip"] = ip_mother_peer_number;
            rocksdb_j["online"] = true;
            rocksdb_j["server"] = true;
            rocksdb_j["fullnode"] = true;
            rocksdb_j["hash_email"] = message_j["hash_of_email"];
            rocksdb_j["prev_hash"] = cred["prev_hash"];
            std::string hash_email = cred["email_hashed"];
            std::string prev_hash = cred["prev_hash"];
            std::string email_prev_hash_app = hash_email + prev_hash; // TODO should this anonymization not be numbers instead of strings?
            std::string full_hash = crypto.bech32_encode_sha256(email_prev_hash_app);
            rocksdb_j["full_hash"] = full_hash;
            rocksdb_j["block_nr"] = proto.get_last_block_nr();
            pl.handle_print_or_log({"get last block number from the first block: ", rocksdb_j["block_nr"]});
            rocksdb_j["ecdsa_pub_key"] = message_j["ecdsa_pub_key"];
            rocksdb_j["rsa_pub_key"] = message_j["rsa_pub_key"];

            message_j["rocksdb"] = rocksdb_j;
            std::string message = message_j.dump();

            P2pNetwork pn;
            for (;;) // new_co must be able to run multiple times
            {
                pn.p2p_client(ip_mother_peer, message);
                if (pn.get_closed_client() == "closed_conn")
                {
                    // TODO must be implemented

                    break;
                }
                else if (pn.get_closed_client() == "close_same_conn")
                {
                    pl.handle_print_or_log({"Connection was closed, probably no server reachable, maybe you are genesis"});

                    // you are the only peer (genesis) and can create a block

                    merkle_tree mt;
                    std::string prev_hash = mt.get_genesis_prev_hash();
                    std::string email_prev_hash_app = hash_email + prev_hash; // TODO should this anonymization not be numbers instead of strings?
                    std::string full_hash = crypto.bech32_encode_sha256(email_prev_hash_app);

                    to_block_j["full_hash"] = full_hash;
                    to_block_j["ecdsa_pub_key"] = cred["ecdsa_pub_key"];
                    to_block_j["rsa_pub_key"] = cred["rsa_pub_key"];

                    std::shared_ptr<std::stack<std::string>> s_shptr = make_shared<std::stack<std::string>>();
                    s_shptr->push(to_block_j.dump());
                    s_shptr = mt.calculate_root_hash(s_shptr);
                    entry_tx_j["full_hash"] = to_block_j["full_hash"];
                    entry_tx_j["ecdsa_pub_key"] = to_block_j["ecdsa_pub_key"];
                    entry_tx_j["rsa_pub_key"] = to_block_j["rsa_pub_key"];
                    entry_transactions_j.push_back(entry_tx_j);
                    exit_tx_j["full_hash"] = "";
                    exit_transactions_j.push_back(exit_tx_j);
                    std::string datetime = mt.time_now();
                    std::string root_hash_data = s_shptr->top();
    pl.handle_print_or_log({"root_hash_data:", root_hash_data});
                    nlohmann::json block_j = mt.create_block(datetime, root_hash_data, entry_transactions_j, exit_transactions_j);
                    std::string block_nr = proto.get_last_block_nr();
                    std::string block_temp_s = mt.save_block_to_file(block_j, block_nr);

                    message_j["full_hash"] = full_hash;
                    message_j["prev_hash"] = prev_hash;
                    message_j["rocksdb"]["full_hash"] = full_hash;
                    message_j["rocksdb"]["prev_hash"] = prev_hash;

                    // fill the matrices
                    Poco::BlockMatrix bm;
                    bm.add_block_to_block_vector(block_j);
                    bm.add_block_vector_to_block_matrix();
                    bm.add_calculated_hash_to_calculated_hash_vector(block_j);
                    bm.add_calculated_hash_vector_to_calculated_hash_matrix();
                    bm.add_prev_hash_to_prev_hash_vector(block_j);
                    bm.add_prev_hash_vector_to_prev_hash_matrix();
                    Poco::IntroMsgsMat imm;
                    imm.add_intro_msg_to_intro_msg_s_vec(message_j);
                    imm.add_intro_msg_s_vec_to_intro_msg_s_2d_mat();
                    imm.add_intro_msg_s_2d_mat_to_intro_msg_s_3d_mat();
                    Poco::IpHEmail ihe;
                    ihe.add_ip_hemail_to_ip_hemail_vec(ip_mother_peer_number, hash_email); // TODO you have to reset this
                    Poco::IpAllHashes iah;
                    iah.add_ip_hemail_to_ip_all_hashes_vec(ihe.get_all_ip_hemail_vec().at(0));
                    iah.add_ip_all_hashes_vec_to_ip_all_hashes_2d_mat();
                    iah.add_ip_all_hashes_2d_mat_to_ip_all_hashes_3d_mat();
                    ihe.reset_ip_hemail_vec();

                    rocksdb_j["prev_hash"] = prev_hash; // TODO might as well be block_j["prev_hash"] ?!?
                    rocksdb_j["full_hash"] = full_hash;

                    // Update rocksdb
                    std::string rocksdb_s = rocksdb_j.dump();
                    Rocksy* rocksy = new Rocksy("usersdb");
                    rocksy->Put(full_hash, rocksdb_s);
                    delete rocksy;
                    pl.handle_print_or_log({"Rocksdb updated and server started"});

                    // Save the full_hash to file
                    FullHash fh;
                    fh.save_full_hash_to_file(full_hash);

                    // Save the prev_hash to file
                    PrevHash ph;
                    ph.save_my_prev_hash_to_file(prev_hash);

                    break;
                }
                else if (pn.get_closed_client() == "close_this_conn_and_create")
                {
                    pl.handle_print_or_log({"Connection closed by other server, start this server (p2p) and create"});

                    break;
                }
                else if (pn.get_closed_client() == "close_this_conn")
                {
                    pl.handle_print_or_log({"Connection closed by other server, start this server (p2p)"});

                    break;
                }
                else if (pn.get_closed_client() == "new_co")
                {
                    uint32_t ip_new_co = pn.get_ip_new_co();
                    P2p p2p;
                    std::string ip_new_co_s;
                    p2p.number_to_ip_string(ip_new_co, ip_new_co_s);

                    message_j["ip"] = ip_new_co;
                    rocksdb_j["ip"] = ip_new_co;
                    message_j["rocksdb"] = rocksdb_j;
                    message = message_j.dump();

                    ip_mother_peer = ip_new_co_s;

                    pl.handle_print_or_log({"The p2p_client did it's job and the new_co too"});
                }
                else
                {
                    // Is this else necessary, looks like a fallback ...
                    pl.handle_print_or_log({"The p2p_client did it's job succesfully"});

                    break;
                }
            }

            if (pn.get_closed_client() != "close_this_conn")
            {
                Poco::Synchronisation* sync = new Poco::Synchronisation();
                sync->get_sleep_and_create_block();
            }

            return true;
        }
        else
        {
            delete rocksy;

            pl.handle_print_or_log({"1 or more peers ..."});

            // 1 or more peers ...
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

        // pl.handle_print_or_log({"Existing peer"});

        // verify if the email address with the saved prev_hash gives the full_hash, otherwise return false
        // verify blockchain ...
        // p2p_client with {"full_hash": "xxxx", "online": "true", "latest_block_nr": "xxx"} to 'try' FindNextPeer() and update blockchain and rocksy
        // then p2p_server()

        Verification verification;
        std::string email_address = cred["email"];
        if (verification.compare_email_with_saved_full_hash(email_address) && verification.verify_all_blocks())
        {
            //ok, continue

            Rocksy* rocksy = new Rocksy("usersdbreadonly");
            FullHash fh;
            std::string my_full_hash = fh.get_full_hash_from_file();

            for (int i = 0; i < 100; i++)
            {
                std::string full_hash_peer = rocksy->FindNextPeer(my_full_hash);
                nlohmann::json contents_j = nlohmann::json::parse(rocksy->Get(full_hash_peer));
                
                uint32_t ip = contents_j["ip"];
                std::string ip_next_peer;
                P2p p2p;
                p2p.number_to_ip_string(ip, ip_next_peer);

                nlohmann::json message_j, to_sign_j;
                message_j["req"] = "intro_online";
                message_j["full_hash"] = my_full_hash; // TODO should be static set up in auth.hpp
                Protocol protocol;
                message_j["latest_block_nr"] = protocol.get_last_block_nr();
                message_j["ip"] = ip;
                message_j["server"] = true;
                message_j["fullnode"] = true;

                to_sign_j["req"] = message_j["req"];
                to_sign_j["full_hash"] = message_j["full_hash"];
                to_sign_j["latest_block_nr"] = message_j["latest_block_nr"];
                to_sign_j["ip"] = message_j["ip"];
                to_sign_j["server"] = message_j["server"];
                to_sign_j["fullnode"] = message_j["fullnode"];
                std::string to_sign_s = to_sign_j.dump();

                Common::Crypto crypto;
                ECDSA<ECP, SHA256>::PrivateKey private_key;
                std::string signature;
                crypto.ecdsa_load_private_key_from_string(private_key);
                if (crypto.ecdsa_sign_message(private_key, to_sign_s, signature))
                {
                    message_j["signature"] = crypto.base64_encode(signature);
                }
                std::string message_s = message_j.dump();
pl.handle_print_or_log({"intro online message sent to", ip_next_peer});
                P2pNetwork pn;
                if (pn.p2p_client(ip_next_peer, message_s) == 0)
                {
                    my_full_hash = full_hash_peer;
                    continue;
                }
                else
                {
                    break;
                }
            }

            delete rocksy;
        }
        else
        {
            //not ok
            return false;
        }
    }
    else
    {
        // something wrong cred["new_peer"] not present or correct
        pl.handle_print_or_log({"Wrong cred[\"new_peer\"]"});

        return false;
    }
     
    return true;
}

int P2p::ip_string_to_number (const char* pDottedQuad, unsigned int &pIpAddr)
{
    unsigned int            byte3;
    unsigned int            byte2;
    unsigned int            byte1;
    unsigned int            byte0;
    char              dummyString[2];

    /* The dummy string with specifier %1s searches for a non-whitespace char
    * after the last number. If it is found, the result of sscanf will be 5
    * instead of 4, indicating an erroneous format of the ip-address.
    */
    if (sscanf (pDottedQuad, "%u.%u.%u.%u%1s", &byte3, &byte2, &byte1, &byte0, dummyString) == 4)
    {
        if ((byte3 < 256) && (byte2 < 256) && (byte1 < 256) && (byte0 < 256))
        {
            pIpAddr  = (byte0 << 24)
                     + (byte1 << 16)
                     + (byte2 << 8)
                     +  byte3;

            Common::Print_or_log pl;
            pl.handle_print_or_log({"Here could big endian be introduced ...", std::to_string(pIpAddr)});
            return 1;
        }
    }

    Common::Print_or_log pl;
    pl.handle_print_or_log({"Here could big endian be introduced ...", std::to_string(pIpAddr)});
    return 0;
}

int P2p::number_to_ip_string(enet_uint32 ipAddress, std::string& ip_string)
{
    char ipAddr[16];
    if (ipAddress) {
        snprintf(ipAddr,sizeof ipAddr,"%u.%u.%u.%u" ,(ipAddress & 0x000000ff) 
                                                    ,(ipAddress & 0x0000ff00) >> 8
                                                    ,(ipAddress & 0x00ff0000) >> 16
                                                    ,(ipAddress & 0xff000000) >> 24);
    }
    ip_string = ipAddr;

    return 0;
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