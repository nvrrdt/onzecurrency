#include "poco.hpp"
#include "p2p.hpp"
#include "auth.hpp"
#include "crypto.hpp"
#include "p2p_network.hpp"

using namespace Crowd;

void Poco::inform_chosen_ones(std::string my_last_block_nr, nlohmann::json block_j, std::map<enet_uint32, std::string> &all_full_hashes)
{
    Auth a;
    std::string my_full_hash = a.get_my_full_hash();
    Crypto* crypto = new Crypto();
    std::string hash_of_block = block_j["prev_hash"]; // TODO: should be full_hash_co? or hash of block_j?________________
    delete crypto;
    Rocksy* rocksy = new Rocksy();
    std::string co_from_this_block = rocksy->FindChosenOne(hash_of_block);
    delete rocksy;

    if (co_from_this_block == my_full_hash)
    {
        // You are the coordinator!
        std::cout << "Inform my fellow chosen_ones as coordinator" << std::endl;

        Protocol proto;
        std::map<int, std::string> parts = proto.partition_in_buckets(my_full_hash, my_full_hash);

        nlohmann::json message_j, to_sign_j; // maybe TODO: maybe you should communicate the partitions, maybe not
        message_j["req"] = "intro_block";
        message_j["latest_block_nr"] = my_last_block_nr;
        message_j["block"] = block_j;
        message_j["prev_hash"] = hash_of_block;
        message_j["full_hash_coord"] = my_full_hash;

        std::string k, v;
        for (auto &[k, v] : parts)
        {
            message_j["chosen_ones"][k] = v;
        }

        to_sign_j["last_block_nr"] = my_last_block_nr;
        to_sign_j["block"] = block_j;
        to_sign_j["prev_hash"] = hash_of_block;
        to_sign_j["full_hash_coord"] = my_full_hash;
        to_sign_j["chosen_ones"] = message_j["chosen_ones"];
        std::string to_sign_s = to_sign_j.dump();
        ECDSA<ECP, SHA256>::PrivateKey private_key;
        std::string signature;
        Crypto* crypto = new Crypto();
        crypto->ecdsa_load_private_key_from_string(private_key);
        if (crypto->ecdsa_sign_message(private_key, to_sign_s, signature))
        {
            message_j["signature"] = crypto->base64_encode(signature);
        }
        delete crypto;

        P2pNetwork pn;
        std::string key, val;
        for (auto &[key, val] : parts)
        {
            if (val == my_full_hash || val == "" || val == "0") continue; // UGLY: sometimes it's "" and sometimes "0" --> should be one or the other

            Rocksy* rocksy = new Rocksy();

            // lookup in rocksdb
            nlohmann::json value_j = nlohmann::json::parse(rocksy->Get(val));
            uint32_t peer_ip = value_j["ip"];
            message_j["rocksdb"] = value_j;

            delete rocksy;
            
            std::string message = message_j.dump();

            for (auto &[key, value] : all_full_hashes)
            {
                std::cout << "Preparation for intro_block: " << peer_ip << " " << to_string(key) << std::endl;
                if (peer_ip != key)
                {
                    std::string peer_ip_from_key;
                    P2p p2p;
                    p2p.number_to_ip_string(key, peer_ip_from_key);

                    // p2p_client() to all chosen ones with intro_peer request
                    pn.p2p_client(peer_ip_from_key, message);
                }
            }
        }
    }
    else
    {
        // You're not the coordinator!
        std::cout << "You're not the coordinator!" << std::endl;
    }
}

// the block still needs to be hashed and the hash sent
// at receiver of intro_block side the block needs to be compared with the list of new_peers the receiver has
// we need to verify what decides the coordinator role, the hash is based on the txs or the complete block?
// who decides you're the coordinator? verify again!